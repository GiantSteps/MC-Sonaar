//
//  Essentia.h
//  pd_essentia~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//

#ifndef __pd_essentia___Essentia__
#define __pd_essentia___Essentia__

#include <iostream>
#include <map>

#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/pool.h>
#include <essentia/streaming/streamingalgorithm.h>
#include <essentia/streaming/algorithms/vectorinput.h>
#include <essentia/streaming/algorithms/vectoroutput.h>
#include <essentia/streaming/algorithms/ringbufferinput.h>
#include <essentia/streaming/algorithms/ringbufferoutput.h>

#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/scheduler/network.h>
#include <memory>
#include <essentia/configurable.h>
#include <essentia/debugging.h>


#include "flext.h"


using namespace std;
using namespace essentia;
using namespace essentia::streaming;

class Essentia  {
public:
    Essentia();
    ~Essentia();
    
    void setup(int sampleRate, int frameSize, int hopSize);
    void compute(vector<Real>& audioFrame,vector<Real>& output);

    
//    vector<flext::AtomList> getFeatures();
//    void getFeatures();
    std::map<string, vector<Real> > getFeatures();
    
    //////ALGS////////
    essentia::streaming::Algorithm *w,*spectrum,*triF,*superFluxF,*superFluxP, * fc;
    Sink<vector<Real> > _output;
    /////// PARAMS //////////////
    int sampleRate, frameSize, hopSize;
    
    /////STRUCTURES//////
    
    streaming::Algorithm* gen;
    streaming::RingBufferOutput* essout;
    
    
    vector<Real> output;
    scheduler::Network *network;
    
    Pool pool;
    
    int framecount;
};

#endif /* defined(__pd_essentia___Essentia__) */
