//
//  EssentiaOnset.h
//  essentiaRT~
//
//  Created by martin hermant on 27/05/14.
//
//

#ifndef __essentiaRT___EssentiaOnset__
#define __essentiaRT___EssentiaOnset__

#include <iostream>


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

using namespace std;
using namespace essentia;
using namespace streaming;

class EssentiaOnset {
    
public:

    EssentiaOnset();
    ~EssentiaOnset();
    
    
    void setup(int frameS,int hopS,int sR,Pool& poolin,Real threshold);
    
    float compute(vector<Real>& audioFrameIn, vector<Real>& output);
    
    
//private:
    
    /// ESSENTIA
    /// algos
    essentia::streaming::Algorithm *w,*spectrum,*triF,*superFluxF,*superFluxP, * fc,*centroidF,*mfccF,*pspectrum;
    
    //// IO
    streaming::Algorithm* gen;
    streaming::RingBufferOutput* essout;
    streaming::RingBufferOutput* DBGOUT;
    streaming::VectorOutput< vector<Real> > * probe;
    
    
    Pool* pool;
    
    
    scheduler::Network *network;
    int sampleRate, frameSize, hopSize;
    
    vector<Real> strength;
    


};

#endif /* defined(__essentiaRT___EssentiaOnset__) */
