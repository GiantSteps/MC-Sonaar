//
//  EssentiaSFX.cpp
//  essentiaRT~
//
//  Created by martin hermant on 27/05/14.
//
//

#include "EssentiaSFX.h"




EssentiaSFX::~EssentiaSFX(){
    delete network;
}
EssentiaSFX::EssentiaSFX(){
    
}

void EssentiaSFX::setup(int fS,int hS,int sR){
    this->sampleRate = sR;
    this->frameSize = fS;
    this->hopSize = hS;
    //this->outPool = poolout;
    
    
    // Instanciate
     AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();
    standard::AlgorithmFactory& stfactory = standard::AlgorithmFactory::instance();
        // Input
    gen = (essentia::streaming::RingBufferInput*)factory.create("RingBufferInput","bufferSize",hopSize*2,"blockSize",hopSize);

    
    fc = factory.create("FrameCutter",
                        "frameSize",frameSize,
                        "hopSize",hopSize,
                        "startFromZero" , true,
                        "validFrameThresholdRatio", 1,
                        "lastFrameToEndOfFile",true,
                        "silentFrames","drop"
                        );
    
    w = factory.create("Windowing","Normalize",false);
    
        // Core
    loudness = factory.create("Loudness");
    
    spectrum = factory.create("Spectrum"); 

    flatness  = factory.create("Flatness");

    yin = factory.create("PitchYinFFT");
    
    cent = factory.create("Centroid");
    mfcc = factory.create("MFCC");
    
        // Aggregation
    const char* statsToCompute[] = {"mean", "var"};
    poolAggr = stfactory.create("PoolAggregator","defaultStats",arrayToVector<string>(statsToCompute));
    
    
 
    
    // Connect
    gen->output("signal") >> fc->input("signal");
    fc->output("frame") >> w->input("frame");
    w->output("frame") >> spectrum->input("frame");
    
    
    // pitchiness
    spectrum->output("spectrum") >> flatness->input("array");

    
    //loudness
    fc->output("frame") >> loudness->input("signal");
    
    
    // f0 yin
    spectrum->output("spectrum") >> yin->input("spectrum");//,"maxFrequency",8000);

    
    //mfcc
    spectrum->output("spectrum") >> mfcc->input("spectrum");
    mfcc->output("bands")>>DEVNULL;
    
    
    
    //Connect SFX 2 Pool
    flatness->output("flatness") >> PC(pool,"noisiness");
    loudness->output("loudness") >> PC(pool,"loudness");
    yin->output("pitch") >> PC(pool,"pitch");
    yin->output("pitchConfidence") >> PC(pool,"pitchConfidence");
    mfcc->output("mfcc") >> PC(pool,"mfcc");
    
    
    
    
    //Connect Aggregator
    poolAggr->input("input").set(pool);
    poolAggr->output("output").set(aggrPool);
    

    
    
    
    
    network = new scheduler::Network(gen);
    network->init();
    
}



void EssentiaSFX::clear(){
    
    fc->reset();
    gen->reset();
    pool.clear();
    
    
    // Hack to clean previous buffers
//    disconnect(fc->output("frame"),w->input("frame"));
//    fc->output("frame") >> DEVNULL;
//    network->init();
//    //network->update();
//    network->runStack();
//    
//    disconnect(fc->output("frame"),DEVNULL);
//    fc->output("frame") >> w->input("frame");
//    network->init();
    
}
void EssentiaSFX::compute(vector<Real>& audioFrameIn){
    gen->add(&audioFrameIn[0],audioFrameIn.size());
    gen->process();
    
    network->runStack();
    

    
}

void EssentiaSFX::aggregate(){
    aggrPool.clear();
    preprocessPool();
    poolAggr->compute();

    
    
}


void EssentiaSFX::preprocessPool(){
    
}