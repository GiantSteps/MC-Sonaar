//
//  EssentiaPD.cpp
//  pd_essentia~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//

#include "EssentiaPD.h"

EssentiaPD::EssentiaPD()
{

}
EssentiaPD::~EssentiaPD()
{
    
    //        delete audio;
    delete fc;
    delete w;
    delete spec;
    delete mfcc;
    delete loud;
    delete pm;
    delete od;
    
    essentia::shutdown();
}

void EssentiaPD::setup(int sampleRate, int frameSize, int hopSize)
{

    essentia::init();
    
    this->sampleRate = sampleRate;
    this->frameSize = frameSize;
    this->hopSize = hopSize;
    
    for(int i=0; i<(frameSize*4); i++)
        frameBuffer.push_back(0.0f);

    // register the algorithms in the factory(ies)
    // we want to compute the MFCC of a file: we need the create the following:
    // audioloader -> framecutter -> windowing -> FFT -> MFCC
    
    AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
    
    //        Algorithm* audio = factory.create("MonoLoader",
    //                                          "filename", audioFilename,
    //                                          "sampleRate", sampleRate);
    
    fc    = factory.create("FrameCutter",
                                      "frameSize", frameSize,
                                      "hopSize", hopSize);
    
    w     = factory.create("Windowing",
                                      "type", "blackmanharris62");
    
    spec  = factory.create("Spectrum");

    mfcc  = factory.create("MFCC");
    loud = factory.create("Loudness");
    
    pm = factory.create("PredominantMelody");

    fft = factory.create("FFT"); //Complex FFT for onset
    c2p = factory.create("CartesianToPolar");
    od = factory.create("OnsetDetection", "method", "complex");
    o = factory.create("Onsets");
    
    FLEXT_ADDTIMER(onsetTimer,m_timerA);
}

void EssentiaPD::compute(vector<Real> audioFrame)
{
//    fc->input("signal").set(audioFrame);

//    // compute a frame
//    fc->compute();
//    
////    // if it was the last one (ie: it was empty), then we're done.
////    if (!frame.size()) {
////        break;
////    }
////    
////    // if the frame is silent, just drop it and go on processing
////    if (isSilent(frame)) continue;
//    
//    w->compute();
    
    //Simple buffer shifting, prob simpler
    onsetDetected = false;
    
    // shift signal buffer contents back.
	for(int i=0; i<frameBuffer.size()-frameSize; i++)
		frameBuffer[i] = frameBuffer[i+frameSize];
	// write new block to end of signal buffer.
	for(int i=0; i<frameSize; i++)
		frameBuffer[frameBuffer.size()-frameSize+i] = audioFrame[i];
    
    pool.clear();
    
    // FrameCutter -> Windowing -> Spectrum
    vector<Real> frame, windowedFrame;

    // Spectrum -> MFCC
    vector<Real> spectrum, mfccCoeffs, mfccBands;
    
    //Loudness
    Real loudness;
    
    //Pitch
    vector<Real> pitch;
    
    //Onsets
    vector<Real> mag, phase;
    vector<complex<Real> > transform;
    
    Real onsetDetection;
    
    fc->input("signal").set(audioFrame);
    
    fc->output("frame").set(frame);
    w->input("frame").set(windowedFrame);
    
    //Window
    w->input("frame").set(audioFrame);
    w->output("frame").set(windowedFrame);

    //Spectrum
    spec->input("frame").set(windowedFrame);
    spec->output("spectrum").set(spectrum);

    //MFCC
    mfcc->input("spectrum").set(spectrum);
    mfcc->output("bands").set(mfccBands);
    mfcc->output("mfcc").set(mfccCoeffs);
    
    //Loudness
    loud->input("signal").set(audioFrame);
    loud->output("loudness").set(loudness);

    //Pitch
    pm->input("signal").set(audioFrame);
    pm->output("pitch").set(pitch);
    
    //Onsets
    fft->input("frame").set(windowedFrame);
    fft->output("fft").set(transform);
    
    c2p->input("complex").set(transform);
    c2p->output("magnitude").set(mag);
    c2p->output("phase").set(phase);
    
    od->input("spectrum").set(spectrum);
    od->input("phase").set(phase);
    od->output("onsetDetection").set(onsetDetection);
    
    
    
    w->compute();
    spec->compute();

    if(currentAlgorithms["spectrum"] || currentAlgorithms["all"])
        pool.add("lowlevel.spec", spectrum);
    
    if(currentAlgorithms["mfcc"] || currentAlgorithms["all"]) {
        mfcc->compute();
        pool.add("lowlevel.mfcc", mfccCoeffs);
    }
    if(currentAlgorithms["loudness"] || currentAlgorithms["all"]) {
        loud->compute();
        pool.add("loudness", loudness);
    }
//    if(currentAlgorithms["melody"] || currentAlgorithms["all"])
//        pm->compute();
    
    //Do the onset work
    if(currentAlgorithms["onsets"] || currentAlgorithms["all"]) {
        fft->compute();
        c2p->compute();
        od->compute();
        
    
        //If we have detected an onset start the time before allowing detection again
        if(onsetDetection >= 3.0 && onsetDetected == false) {
            onsetDetected = true;
            onsetTimer.Delay(50);
        }
        
//        if(onsetDetection > 0.0f)
//            std::cout << onsetDetection << "\n";
        
        vector<Real> onsetDetections;
        onsetDetections.push_back(onsetDetection);
        
        TNT::Array2D<Real> detections;
        detections = TNT::Array2D<Real>(1, onsetDetections.size());
        
        for (int i=0; i < onsetDetections.size(); i++) {
            detections[0][i] = onsetDetections[i];
        }
        
        vector<Real> weights(1);
        weights[0] = 1.0f;
        
        vector<Real> onsetTimes;
        
        o->input("detections").set(detections);
        o->input("weights").set(weights);
        o->output("onsets").set(onsetTimes);
    }
        
    
//        pool.add("melody", pitch);
    //IF WE GOT AN ONSET -> OUTPUT FEATURES
    
}

//vector<flext::AtomList> EssentiaPD::getFeatures()
//{
//    
//
//}

std::map<string, vector<Real> > EssentiaPD::getFeatures()
{
    std::map<string, vector<vector<Real> > >  vectorsIn =     pool.getVectorRealPool();
    std::map<string, vector<Real> > vectorsOut;
    
    for(std::map<string, vector<vector<Real> > >::iterator iter = vectorsIn.begin(); iter != vectorsIn.end(); ++iter)
    {
        string k =  iter->first;
        vector<Real> v = (iter->second)[0];
        
        vectorsOut[k] = v;
    }
    
    std::map<string, vector<Real > >  realsIn =  pool.getRealPool();
    
    for(std::map<string, vector<Real > >::iterator iter = realsIn.begin(); iter != realsIn.end(); ++iter)
    {
        string k =  iter->first;
        vector<Real> v = iter->second;
        
        vectorsOut[k] = v;
    }
    
    return vectorsOut;
}