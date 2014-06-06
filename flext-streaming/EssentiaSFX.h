//
//  EssentiaSFX.h
//  essentiaRT~
//
//  Created by martin hermant on 27/05/14.
//
//

#ifndef __essentiaRT___EssentiaSFX__
#define __essentiaRT___EssentiaSFX__

#include <iostream>




#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/pool.h>
#include <essentia/streaming/streamingalgorithm.h>
#include <essentia/streaming/algorithms/vectorinput.h>
#include <essentia/streaming/algorithms/vectoroutput.h>
#include <essentia/streaming/algorithms/ringbufferinput.h>

#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/scheduler/network.h>

using namespace std;
using namespace essentia;
using namespace streaming;

class EssentiaSFX {
    
public:
    
    EssentiaSFX();
    ~EssentiaSFX();
    
    
    void setup(int frameS,int hopS,int sR);
    
    void compute(vector<Real>& audioFrameIn);
    
    void aggregate();
    void clear();
    Pool aggrPool;
private:
    
    /// ESSENTIA
    /// algos
    essentia::streaming::Algorithm *w,*spectrum,  * fc;
    
    
    essentia::streaming::Algorithm *loudness,*flatness;
    
    essentia::streaming::Algorithm *cent;
    
    essentia::streaming::Algorithm *yin;
    essentia::streaming::Algorithm *mfcc;

    essentia::standard::Algorithm *poolAggr;
    
    //// IO
    essentia::streaming::RingBufferInput* gen;

   
    
    
    
    
    scheduler::Network *network;
    int sampleRate, frameSize, hopSize;
    
    Pool pool;//,*outPool;
    
    //Pool
    void preprocessPool();
    
    
};






#endif /* defined(__essentiaRT___EssentiaSFX__) */
