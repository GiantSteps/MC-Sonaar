//
//  Essentia.cpp
//  pd_essentia~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//

#include "Essentia.h"

Essentia::Essentia()
{

}
Essentia::~Essentia()
{
    
    //        delete audio;
    delete fc;
    delete w;
    delete spec;
    delete mfcc;
    delete loud;
    
    essentia::shutdown();
}

void Essentia::setup(int sampleRate, int frameSize, int hopSize)
{

    essentia::init();
    
    this->sampleRate = sampleRate;
    this->frameSize = frameSize;
    this->hopSize = hopSize;
    

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
}

void Essentia::compute(vector<Real> audioFrame)
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
    
    // FrameCutter -> Windowing -> Spectrum
    vector<Real> frame, windowedFrame;
    
    // Spectrum -> MFCC
    vector<Real> spectrum, mfccCoeffs, mfccBands;
    
    //Loudness
    Real loudness;
    
    pool.clear();

    spec->input("frame").set(audioFrame);
    spec->output("spectrum").set(spectrum);

    mfcc->input("spectrum").set(spectrum);
    mfcc->output("bands").set(mfccBands);
    mfcc->output("mfcc").set(mfccCoeffs);
    
    loud->input("signal").set(audioFrame);
    loud->output("loudness").set(loudness);
    
    //
    spec->compute();
    mfcc->compute();
    loud->compute();
    
    pool.add("lowlevel.mfcc", mfccCoeffs);
    pool.add("lowlevel.spec", spectrum);
    pool.add("loudness", loudness);
    
    //IF WE GOT AN ONSET -> OUTPUT FEATURES
    
}

//vector<flext::AtomList> Essentia::getFeatures()
//{
//    
//
//}

std::map<string, vector<Real> > Essentia::getFeatures()
{
    std::map<string, vector<vector<Real> > >  vectorsIn =     pool.getVectorRealPool();
    std::map<string, vector<Real> > vectorsOut;
    
    for(std::map<string, vector<vector<Real> > >::iterator iter = vectorsIn.begin(); iter != vectorsIn.end(); ++iter)
    {
        string k =  iter->first;
        vector<Real> v = (iter->second)[0];
        
        vectorsOut[k] = v;
    }
    return vectorsOut;
}