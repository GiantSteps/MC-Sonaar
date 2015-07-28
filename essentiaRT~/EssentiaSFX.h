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
#include <thread>



#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/pool.h>
#include <essentia/streaming/streamingalgorithm.h>
#include <essentia/streaming/algorithms/vectorinput.h>
#include <essentia/streaming/algorithms/vectoroutput.h>
#include <essentia/streaming/algorithms/ringbufferinput.h>

#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/scheduler/network.h>

#define CROP_LAST_FRAME 6

using namespace std;
using namespace essentia;
using namespace streaming;

class EssentiaSFX {
    
public:
    EssentiaSFX(){};
    EssentiaSFX(int frameS,int hopS,int sR);
    ~EssentiaSFX();
    
    
    void setup(int frameS,int hopS,int sR);
    
    void compute(vector<Real>& audioFrameIn);
    //Pool
    void preprocessPool();
    
    void aggregate();
    void clear();
    Pool aggrPool,sfxPool;
    int frameSize, hopSize;
private:
    
    /// ESSENTIA
    /// algos
    essentia::streaming::Algorithm *w,*spectrum,  * fc, *env;
    
    
    essentia::streaming::Algorithm *loudness,*flatness;
    
    essentia::streaming::Algorithm *cent;
    
    essentia::streaming::Algorithm *yin;
    essentia::streaming::Algorithm *mfcc;
    essentia::streaming::Algorithm *TCent;
    essentia::streaming::Algorithm *spectralPeaks,*hpcp;

    essentia::standard::Algorithm *poolAggr;
    
    
    //// IO
    essentia::streaming::RingBufferInput* gen;

   
    
    
    
    
    scheduler::Network *network = NULL;
    int sampleRate;
    
    
    


    
    
};






#endif /* defined(__essentiaRT___EssentiaSFX__) */
