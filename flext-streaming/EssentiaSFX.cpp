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

void EssentiaSFX::setup(int fS,int hS,int sR,Pool* poolout){
    this->sampleRate = sR;
    this->frameSize = fS;
    this->hopSize = hS;
    //this->outPool = poolout;
    
    
    // Instanciate
     AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();
    standard::AlgorithmFactory& stfactory = standard::AlgorithmFactory::instance();
        // Input
    gen = (essentia::streaming::RingBufferInput*)factory.create("RingBufferInput","bufferSize",frameSize*10,"blockSize",hopSize);

    
    fc = factory.create("FrameCutter",
                        "frameSize",frameSize,
                        "hopSize",hopSize,
                        "startFromZero" , true,
                        "validFrameThresholdRatio", 1,
                        "lastFrameToEndOfFile",true);
    
    w = factory.create("Windowing","Normalize",false);
    
        // Core
    loudness = factory.create("Loudness");
    
    spectrum = factory.create("Spectrum"); 

    flatness  = factory.create("Flatness");

    yin = factory.create("PitchYinFFT");
    
    cent = factory.create("Centroid");
    
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

    
    
    
    //Connect SFX 2 Pool
    flatness->output("flatness") >> PC(pool,"pitched");
    loudness->output("loudness") >> PC(pool,"loudness");
    yin->output("pitch") >> PC(pool,"pitch");
    yin->output("pitchConfidence") >> PC(pool,"pitchConfidence");
    
    
    
    
    
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
    
    poolAggr->compute();
//    vector<string> f = aggrPool.descriptorNames();
//    for ( vector<string>::iterator it=f.begin();it != f.end();++it){
//        outPool->set(*it, aggrPool.value<Real>(*it));
//    }
    
    
}