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
//    delete fc;
    delete w;
    delete spec;
    delete mfcc;
    delete loud;
    delete pm;
    
    delete fft;
    delete c2p;
    delete od;
    delete o;
    
    delete el;
//    delete pm;
    
    essentia::shutdown();
}

void EssentiaPD::setup(int sampleRate, int frameSize, int hopSize)
{    
    essentia::init();

    
    this->sampleRate = sampleRate;
    this->frameSize = frameSize;
    this->hopSize = hopSize;
    
    int noOfFramesInBuffer = 1;
    int frameBufferSize = frameSize * noOfFramesInBuffer;
    
    //Keep a history of four frames
    for(int i=0; i<(frameBufferSize); i++)
        frameBuffer.push_back(0.0f);
    
    latestFrameTime = (float)frameBufferSize / 44100.0;

    // register the algorithms in the factory(ies)
    // we want to compute the MFCC of a file: we need the create the following:
    // audioloader -> framecutter -> windowing -> FFT -> MFCC
    
    AlgorithmFactory &factory = standard::AlgorithmFactory::instance();
    
    //        Algorithm* audio = factory.create("MonoLoader",
    //                                          "filename", audioFilename,
    //                                          "sampleRate", sampleRate);
    
    
    w     = factory.create("Windowing",
                                      "type", "blackmanharris62");
    
    spec  = factory.create("Spectrum");

    mfcc  = factory.create("MFCC");
    loud = factory.create("Loudness");
    
    el = factory.create("EqualLoudness");
    pm = factory.create("PredominantMelody",
                        "frameSize", frameSize,
                        "hopSize", hopSize,
                        "sampleRate", sampleRate);

    fft = factory.create("FFT"); //Complex FFT for onset
    c2p = factory.create("CartesianToPolar");
    od = factory.create("OnsetDetection", "method", "complex");
    o = factory.create("Onsets");
    
    FLEXT_ADDTIMER(onsetTimer,m_timerA);
}



void EssentiaPD::compute(const vector<Real>& audioFrameIn)
{
    pool.clear();
    
    //Simple circular buffer thing
    
    // shift signal buffer contents back.
	for(int i=0; i<frameBuffer.size()-frameSize; i++)
		frameBuffer[i] = frameBuffer[i+frameSize];
	// write new block to end of signal buffer.
	for(int i=0; i<frameSize; i++)
		frameBuffer[frameBuffer.size()-frameSize+i] = audioFrameIn[i];
    
    AlgorithmFactory &factory = standard::AlgorithmFactory::instance();
    
    Algorithm *fc    = factory.create("FrameCutter",
                           "frameSize", frameSize,
                           "hopSize", hopSize);
    
    // FrameCutter -> Windowing -> Spectrum
    vector<Real> frame, windowedFrame;
    
    // Spectrum -> MFCC
    vector<Real> spectrum, mfccCoeffs, mfccBands;
    
    //Loudness
    Real loudness;
    
    //Pitch
    vector<Real> pitch, pitchConfidence;
    vector<Real> equalLoudnessSignal;
    
    //Onsets
    vector<Real> mag, phase;
    vector<complex<Real> > transform;
    
    Real onsetDetection;
    
    fc->input("signal").set(frameBuffer);
    fc->output("frame").set(frame);

    //Window
    w->input("frame").set(frame);
    w->output("frame").set(windowedFrame);

    //Spectrum
    spec->input("frame").set(windowedFrame);
    spec->output("spectrum").set(spectrum);

    //MFCC
    mfcc->input("spectrum").set(spectrum);
    mfcc->output("bands").set(mfccBands);
    mfcc->output("mfcc").set(mfccCoeffs);
    
    //Loudness
    loud->input("signal").set(audioFrameIn);
    loud->output("loudness").set(loudness);

    //Pitch
    el->input("signal").set(frameBuffer);
    el->output("signal").set(equalLoudnessSignal);
    pm->input("signal").set(equalLoudnessSignal);
    pm->output("pitch").set(pitch);
    pm->output("pitch").set(pitchConfidence);
    
    //Onsets
    fft->input("frame").set(windowedFrame);
    fft->output("fft").set(transform);
    
    c2p->input("complex").set(transform);
    c2p->output("magnitude").set(mag);
    c2p->output("phase").set(phase);
    
    od->input("spectrum").set(spectrum);
    od->input("phase").set(phase);
    od->output("onsetDetection").set(onsetDetection);
    
    TNT::Array2D<Real> detections;
    vector<Real> onsetDetections;
    
    int i=0;
    while (true) {
        //Get a frame
        fc->compute();
        
        // if it was the last one (ie: it was empty), then we're done.
        if (!frame.size()) {
            break;
        }
    
        // if the frame is silent, just drop it and go on processing
//        if (isSilent(frame)) continue;
    
        w->compute();
        spec->compute();

        
        if(currentAlgorithms["mfcc"] || currentAlgorithms["all"]) {
            mfcc->compute();
        }
        if(currentAlgorithms["loudness"] || currentAlgorithms["all"]) {
            loud->compute();
        }
        
    //    //Not working at the moment
    //    if(currentAlgorithms["melody"] || currentAlgorithms["all"]) {
    //        el->compute();
    //        pm->compute();
    //        pool.add("pitch", pitch);
    //        pool.add("pitchConfidence", pitchConfidence);
    //    }

        
        
        fft->compute();
        c2p->compute();
        od->compute();
        
        onsetDetections.push_back(onsetDetection);
    }

    if(currentAlgorithms["spectrum"] || currentAlgorithms["all"])
        pool.add("spectrum", spectrum);
    
    if(currentAlgorithms["mfcc"] || currentAlgorithms["all"]) {
        pool.add("mfcc", mfccCoeffs);
    }
    if(currentAlgorithms["loudness"] || currentAlgorithms["all"]) {
        pool.add("loudness", loudness);
    }
    
    
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
        
    o->compute();
    
    std::cout << "detection: " << onsetDetection << "\n";
    std::cout << "onsetDetected: " << onsetDetected << "\n";
    
    //Quick and dirty onset detection that does no peak picking
    //just select peak above threshold and start timer
    if(onsetDetection >= 2.0f && onsetDetected == false) {
//        std::map<string, vector<Real> > features = getFeatures();
//        outputListOfFeatures(features);
        
        
        onsetDetected = true;
        
        //takes seconds
        onsetTimer.Delay(0.050);
    }
    
    //Online Peak-picking
    
    
//    //If an onset occurs in this frame trigger output
//    for(int i=0; i<onsetTimes.size(); i++) {
//        if (onsetTimes[i] >= latestFrameTime) {
//            post("onset detected");
//            onsetDetected = true;
//        }
//    }
    
    delete fc;
}



//vector<flext::AtomList> EssentiaPD::getFeatures()
//{
//
//
//}

void EssentiaPD::outputListOfFeatures(const std::map<string, vector<Real> >& features)
{
    for(std::map<string, vector<Real>  >::const_iterator iter = features.begin(); iter != features.end(); ++iter)
    {
        AtomList listOut(iter->second.size()+1);
        t_atom featureName;
        
        SetString(featureName, iter->first.c_str());
        
        listOut[0] = featureName;
        
        for(int i=0; i<iter->second.size(); i++) {
            t_atom featureValue;
            
            SetFloat(featureValue, iter->second[i]);
            
            listOut[i+1] = featureValue;
        }
        
        ToQueueList(1, listOut);
        
        
    }
}

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