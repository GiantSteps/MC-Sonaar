//
//  EssentiaSFX.cpp
//  essentiaRT~
//
//  Created by martin hermant on 27/05/14.
//
//

#include "EssentiaSFX.h"




EssentiaSFX::~EssentiaSFX(){
    
    if(network!=NULL){
        network->clear();
        delete network;
    }
    
    
}
EssentiaSFX::EssentiaSFX(int frameS,int hopS,int sR){
    setup(frameS, hopS, sR);
}

void EssentiaSFX::setup(int fS,int hS,int sR){
    this->sampleRate = sR;
    this->frameSize = fS;
    this->hopSize = hS;
    
    
    ///////////
    // Instanciate
    ///////////
    AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();
    standard::AlgorithmFactory& stfactory = standard::AlgorithmFactory::instance();
    // Input
    gen = new essentia::streaming::RingBufferInput();//factory.create("RingBufferInput","bufferSize",hopSize*2,"blockSize",hopSize);
    gen->_bufferSize = hopSize;
    gen->output(0).setReleaseSize(hopSize);
    gen->output(0).setAcquireSize(hopSize);
    gen->configure();
    
    fc = factory.create("FrameCutter",
                        "frameSize",frameSize,
                        "hopSize",hopSize,
                        "startFromZero" , true,
                        "validFrameThresholdRatio", 0,
                        "lastFrameToEndOfFile",true,
                        "silentFrames","keep"
                        );
    
    w = factory.create("Windowing","zeroPhase",true,"type","square");
    env = factory.create("Envelope");
    
    
        // Core
    loudness = factory.create("InstantPower");
    spectrum = factory.create("Spectrum");
    flatness  = factory.create("Flatness");
    yin = factory.create("PitchYinFFT");
    cent = factory.create("Centroid");
    TCent = factory.create("TCToTotal");
    mfcc = factory.create("MFCC","inputSize",frameSize/2 + 1);
    hpcp = factory.create("HPCP","size",48);
    spectralPeaks = factory.create("SpectralPeaks","sampleRate",sampleRate);
    
    
    // Aggregation
    const char* statsToCompute[] = {"mean", "var"};
    poolAggr = stfactory.create("PoolAggregator","defaultStats",arrayToVector<string>(statsToCompute));
    
    
    
    /////////////
    // Connect
    /////////////
    gen->output("signal") >> fc->input("signal");
    fc->output("frame")  >> w->input("frame");
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
    mfcc->output("bands")>>         DEVNULL;
    
    
    // hpcp
    spectrum->output("spectrum") >> spectralPeaks->input("spectrum");
    spectralPeaks->output("frequencies") >> hpcp->input("frequencies");
    spectralPeaks->output("magnitudes") >> hpcp->input("magnitudes");
    
    // centroid
    spectrum->output("spectrum") >> cent->input("array");
    
    
    //Temporal Centroid
    env->output("signal") >>        TCent->input("envelope");
    
    //Connect SFX 2 Pool
    flatness->output("flatness")>>      PC(sfxPool,"noisiness");
    loudness->output("power")   >>      PC(sfxPool,"loudness");
    yin->output("pitch")        >>      PC(sfxPool,"f0");
    yin->output("pitchConfidence")>>    PC(sfxPool,"f0Confidence");
    mfcc->output("mfcc")        >>      PC(sfxPool,"mfcc");
    cent->output("centroid")    >>      PC(sfxPool,"centroid");
    hpcp->output("hpcp")        >>      PC(sfxPool,"hpcp");
    
    // accumulator algo are directly linked to aggrPool (computes only at the end)
    TCent->output("TCToTotal") >>   PC(aggrPool,"tempCentroid");
    
    
    
    //Connect Aggregator
    poolAggr->input("input").set(sfxPool);
    poolAggr->output("output").set(aggrPool);
    
    network = new scheduler::Network(gen);
    network->initStack();
    
}



void EssentiaSFX::clear(){
    network->reset();
    aggrPool.clear();
    fc->reset();
    gen->reset();
    sfxPool.clear();
    
    
}
void EssentiaSFX::compute(vector<Real>& audioFrameIn){
    if(audioFrameIn.size()!=gen->_bufferSize){
        gen->_bufferSize = audioFrameIn.size();
        gen->output(0).setAcquireSize(audioFrameIn.size());
        gen->output(0).setReleaseSize(audioFrameIn.size());
        gen->configure();
    }
    gen->add(&audioFrameIn[0],audioFrameIn.size());
    gen->process();
    
    network->runStack(false);

    
    
}

void EssentiaSFX::aggregate(){
    
    
    
    
    
    
    poolAggr->compute();
    
    // avoid first onset and empty onsets
    if(aggrPool.getRealPool().size()>0 && aggrPool.value<Real>("loudness.mean")>0){
        // call should stop for accumulator algorithms
        network->runStack(true);
    }
    
    //rescaling values afterward
    aggrPool.set("centroid.mean",aggrPool.value<Real>("centroid.mean")*sampleRate/2);
    aggrPool.set("centroid.var",aggrPool.value<Real>("centroid.var")*sampleRate*sampleRate/4);
    
    
    // normalize mfcc
    if(aggrPool.contains<vector<Real>>("mfcc.mean")){
        vector<Real> v = aggrPool.value<vector<Real> >("mfcc.mean");
        float factor = (frameSize);
        aggrPool.remove("mfcc.mean");
        for(auto& e:v){
            aggrPool.add("mfcc.mean", e*1.0/factor);
        }
        
        
        
        v = aggrPool.value<vector<Real> >("mfcc.var");
        factor*=factor;
        aggrPool.remove("mfcc.var");
        for(auto& e:v){
            aggrPool.add("mfcc.var", e*1.0/factor);
        }
    }
    
    // only one frame was aquired  ( no aggregation but we still want output!)
    else if (sfxPool.contains<vector<vector<Real>> >("mfcc")){

        vector<Real> v = sfxPool.value<vector<vector<Real> > >("mfcc")[0];
        float factor = (frameSize);
        aggrPool.removeNamespace("mfcc");
        aggrPool.remove("mfcc");
        for(auto& e:v){
            aggrPool.add("mfcc.mean", e*1.0/factor);
            aggrPool.add("mfcc.var", 0);
        }
        
                
        v = sfxPool.value<vector<vector<Real>> >("hpcp")[0];
        
        aggrPool.removeNamespace("hpcp");
        aggrPool.remove("hpcp");
        for(auto& e:v){
            
            aggrPool.add("hpcp.mean", e);
            aggrPool.add("hpcp.var", 0);
        }
      
        
        
    }
    
    
    
    
    
}




void EssentiaSFX::preprocessPool(){
    
    
    
    
    
    
}