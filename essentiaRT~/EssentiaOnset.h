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
#include <thread>
#include <unistd.h>
#include <chrono>

#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/pool.h>
#include <essentia/streaming/streamingalgorithm.h>
#include <essentia/streaming/algorithms/vectorinput.h>
#include <essentia/streaming/algorithms/vectoroutput.h>
#include <essentia/streaming/algorithms/ringbufferinput.h>
#include <essentia/streaming/algorithms/ringbufferoutput.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/streaming/accumulatoralgorithm.h>
#include <essentia/scheduler/network.h>



// multiply novelty function and threshold for easier parametriation, the true computation is Not Multiplied, only inputs and outputs
#define NOVELTY_MULT 100.
#define FRAMESIZE 2048

using namespace std;
using namespace chrono;
using namespace essentia;
using namespace streaming;

class EssentiaOnset {
    
public:
    EssentiaOnset(){};
    EssentiaOnset(int frameS,int hopS,int sR,Real threshold);
    ~EssentiaOnset();
    
    void setup(int frameS,int hopS,int sR,Real threshold);
    
    float compute(vector<Real>& audioFrameIn, vector<Real>& output);
    
    void preprocessPool();
    
//private:
    
    /// ESSENTIA
    /// algos
    streaming::Algorithm *w,*spectrum,*triF,*superFluxF, * fc,*centroidF,*mfccF,*pspectrum;
    streaming::Algorithm *superFluxP;
    //// IO
    
//    essentia::streaming::VectorInput<Real,FRAMESIZE>* gen;
    streaming::VectorOutput<vector<Real> >* essout;
    RingBufferInput * gen;
//    RingBufferOutput * essout;
//    streaming::RingBufferOutput* DBGOUT;
//    streaming::VectorOutput< vector<Real> > * probe;
    
    
    Pool pool;
    
    
    scheduler::Network *network=NULL;
    int sampleRate, frameSize, hopSize;
    
    vector<vector<Real> > strength;

    float combineMs;
    unsigned int frameCount;


};

#endif /* defined(__essentiaRT___EssentiaOnset__) */
