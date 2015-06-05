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
    
    ///////////
    // Instanciate
    ///////////
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
    
    w = factory.create("Windowing");
    env = factory.create("Envelope");
    
    
        // Core
    loudness = factory.create("InstantPower");
    spectrum = factory.create("Spectrum");
    flatness  = factory.create("Flatness");
    yin = factory.create("PitchYinFFT");
    cent = factory.create("Centroid");
    TCent = factory.create("TCToTotal");
    mfcc = factory.create("MFCC");
    
        // Aggregation
    const char* statsToCompute[] = {"mean", "var"};
    poolAggr = stfactory.create("PoolAggregator","defaultStats",arrayToVector<string>(statsToCompute));
    
    
 
    /////////////
    // Connect
    /////////////
    gen->output("signal") >> fc->input("signal");
    fc->output("frame") >> w->input("frame");
    w->output("frame") >> spectrum->input("frame");
    gen->output("signal") >> env->input("signal");
    
    
    // noisiness
    spectrum->output("spectrum") >> flatness->input("array");

    
    //loudness
    fc->output("frame") >> loudness->input("array");
    
    
    // f0 yin
    spectrum->output("spectrum") >> yin->input("spectrum");//,"maxFrequency",8000);

    
    //mfcc
    spectrum->output("spectrum") >> mfcc->input("spectrum");
    mfcc->output("bands")>>DEVNULL;
    
    
    // centroid
    spectrum->output("spectrum") >> cent->input("array");
    
    
    //Temporal Centroid
    env->output("signal") >> TCent->input("envelope");
    
    //Connect SFX 2 Pool
    flatness->output("flatness") >> PC(pool,"noisiness");
    loudness->output("power") >> PC(pool,"loudness");
    yin->output("pitch") >> PC(pool,"f0");
    yin->output("pitchConfidence") >> PC(pool,"f0Confidence");
    mfcc->output("mfcc") >> PC(pool,"mfcc");
    cent->output("centroid") >> PC(pool,"centroid");
    
    // accumulator algo are directly linked to aggrPool (computes only at the end)
    TCent->output("TCToTotal") >> PC(aggrPool,"tempCentroid");
    
    
    
    //Connect Aggregator
    poolAggr->input("input").set(pool);
    poolAggr->output("output").set(aggrPool);
    

    
    
    
    
    network = new scheduler::Network(gen);
    network->run();
    
}



void EssentiaSFX::clear(){
    
    fc->reset();
    gen->reset();
    pool.clear();
    
    
}
void EssentiaSFX::compute(vector<Real>& audioFrameIn){
    gen->add(&audioFrameIn[0],audioFrameIn.size());
    gen->process();
    
//    network->runStack();
    

    
}

void EssentiaSFX::aggregate(){
    
    if(pool.getRealPool().size()>0){
    preprocessPool();
    aggrPool.clear();
    poolAggr->compute();
        
        //rescaling values afterward
       aggrPool.set("centroid.mean",aggrPool.value<Real>("centroid.mean")*sampleRate/2);
       aggrPool.set("centroid.var",aggrPool.value<Real>("centroid.var")*sampleRate*sampleRate/4);
        
    }
    
    // avoid first onset and empty onsets
    if(aggrPool.getRealPool().size()>0 && aggrPool.value<Real>("loudness.mean")>0){
        try {
            
            // call should stop for accumulator algorythms
            network->run();
            // reset Algos and set shouldstop=false
            network->reset();
        } catch (EssentiaException) {
            
            
        }


    }
    
}




void EssentiaSFX::preprocessPool(){
    
    // crop to avoid next transient influence, assuming that the onset callback appears after few frames of the transeient
    std::map<string, vector<Real > >  vectorsIn =     pool.getRealPool();

    
    for(std::map<string, vector<Real > >::iterator iter = vectorsIn.begin(); iter != vectorsIn.end(); ++iter)
    {
        
        int finalSize = iter->second.size()-CROP_LAST_FRAME;
        finalSize = std::max(finalSize,1);
        string k =  iter->first;
        iter->second.resize(finalSize);
        //  Nasty hack :
        pool.remove(k);
        for (int i =0 ; i < iter->second.size();i++){pool.add(k, iter->second[i]);};
        
    }
    
    std::map<string, vector<vector<Real> > >  vectorsIn2 =     pool.getVectorRealPool();
    
    
    for(std::map<string, vector<vector<Real> > >::iterator iter = vectorsIn2.begin(); iter != vectorsIn2.end(); ++iter)
    {
        int finalSize = iter->second.size()-CROP_LAST_FRAME;
        finalSize = std::max(finalSize,1);
        string k =  iter->first;
        iter->second.resize(finalSize);
        //  Nasty hack :
        pool.remove(k);
        for (int i =0 ; i < iter->second.size();i++){pool.add(k, iter->second[i]);};
        
    }


    
}