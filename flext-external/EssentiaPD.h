//
//  EssentiaPD.h
//  pd_essentia~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//


#ifndef __pd_essentia___EssentiaPD__
#define __pd_essentia___EssentiaPD__

// include flext header
#include <flext.h>

// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 403)
#error You need at least flext version 0.4.3
#endif

#include <iostream>
#include <map>

#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/pool.h>

using namespace std;
using namespace essentia;
using namespace essentia::standard;

class EssentiaPD : public flext_base{

public:
    FLEXT_HEADER(EssentiaPD, flext_base)
    std::map<string, bool> currentAlgorithms;

    EssentiaPD();
    ~EssentiaPD();
    
    vector<Real> frameBuffer;
    
    void setup(int sampleRate, int frameSize, int hopSize);
    void compute(const vector<Real>& audioFrameIn);
    
//    vector<flext::AtomList> getFeatures();
//    void getFeatures();
    std::map<string, vector<Real> > getFeatures();
    
    //////ALGS////////
//    Algorithm *fc;
    Algorithm *w, *spec, *mfcc, *loud;
    
    Algorithm *pm, *el;
    
    Algorithm *fft, *c2p, *od, *o;
    
    
    /////// PARAMS //////////////
    int sampleRate, frameSize, hopSize;
    
    /////STRUCTURES//////    
    Pool pool;
    
    bool onsetDetected;
    
protected:
    flext::Timer onsetTimer;
    
    //Reset the timer when we reach the appropriate time
    void m_timerA(void *) { onsetDetected = false;} ;
    
private:
    // register timer callbacks
    
    FLEXT_CALLBACK_T(m_timerA) 

};

#endif /* defined(__pd_essentia___EssentiaPD__) */
