//
//  EssentiaOnset.cpp
//  essentiaRT~
//
//  Created by martin hermant on 27/05/14.
//
//

#include "EssentiaOnset.h"



EssentiaOnset::~EssentiaOnset(){
    
    if(network!=NULL){
        network->clear();
        delete network;
    }
}
EssentiaOnset::EssentiaOnset(int frameS,int hopS,int sR,Real threshold){
    setup(frameS, hopS, sR,  threshold);

}

void EssentiaOnset::setup(int fS,int hS,int sR,Real threshold){


    this->sampleRate = sR;
    this->frameSize = fS;
    this->hopSize = hS;
    
    
    combineMs = 50;
    strength.resize(2);
    
    AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();
    
    fc = factory.create("FrameCutter",
                        "frameSize",frameSize,
                        "hopSize",hopSize,
                        "startFromZero" , true,
                        "validFrameThresholdRatio", .1,
                        "lastFrameToEndOfFile",true,
                        "silentFrames","keep"
                        );
    

    
    w = factory.create("Windowing","type","hann");
    
    spectrum = factory.create("Spectrum");
    pspectrum = factory.create("PowerSpectrum");
    triF = factory.create("TriangularBands","log",false);
    
    superFluxF = factory.create("SuperFluxNovelty","binWidth",3,"frameWidth",1);
    superFluxP= factory.create("SuperFluxPeaks","ratioThreshold" , 4,"threshold" ,.7/NOVELTY_MULT
                               ,"pre_max",50,"pre_avg",80,"frameRate", sampleRate*1.0/hopSize,"combine",combineMs
                               ,"activation_slope",true);
    
    superFluxP->input(0).setAcquireSize(1);
    superFluxP->input(0).setReleaseSize(1);

    
    centroidF = factory.create("Centroid");
    
    mfccF = factory.create("MFCC","inputSize",frameSize/2 + 1);

    
    
    gen = new RingBufferInput();
    gen->_bufferSize = frameSize;
    gen->output(0).setAcquireSize(frameSize);
    gen->output(0).setReleaseSize(frameSize);
    gen->configure();

    essout = new essentia::streaming::VectorOutput<vector<Real> >();
    essout->setVector(&strength);

    
//    probe = new streaming::VectorOutput< vector<Real> >();
    
    // cutting, overlapping
    gen->output("signal") >>        fc->input("signal");
    fc->output("frame")  >>         w->input("frame");
    w->output("frame") >>           spectrum->input("frame");
    //w->output("frame") >> pspectrum->input("signal");
    // SuperFlux
    spectrum->output("spectrum") >> triF->input("spectrum");
    triF->output("bands")>>         superFluxF->input("bands");
    superFluxF->output("Differences")  >>superFluxP->input("novelty");
    superFluxP->output("strengths") >>  essout->input("data");
    superFluxP->output("peaks") >> DEVNULL;
    
    // MFCC
    spectrum->output("spectrum") >> mfccF->input("spectrum");

    mfccF->output("bands") >>       DEVNULL;
    
    
    // centroid
    spectrum->output("spectrum") >> centroidF->input("array");

    //2 Pool
   connectSingleValue(centroidF->output("centroid"),pool,"i.centroid");
    connectSingleValue(mfccF->output("mfcc"),pool,"i.mfcc");

    network = new scheduler::Network(gen);
    network->initStack();

//    essentia::setDebugLevel( essentia::EScheduler);
}

// return 0 for activation_slope and >0 for maximum
float EssentiaOnset::compute(vector<Real>& audioFrameIn, vector<Real>& output){
    strength.clear();
    essout->setVector(&strength);
    
    if(audioFrameIn.size()!=gen->_bufferSize){
        gen->_bufferSize = audioFrameIn.size();
        gen->output(0).setAcquireSize(audioFrameIn.size());
        gen->output(0).setReleaseSize(audioFrameIn.size());
        gen->configure();


    }
    
    gen->add(&audioFrameIn[0], audioFrameIn.size());

    gen->process();
    
    
    
    network->runStack(false);
    dynamic_cast<AccumulatorAlgorithm * >(superFluxP)->finalProduce();
    network->runStack(false);


    float val =-1;
    frameCount+= audioFrameIn.size();

    if(frameCount*1000.0>combineMs*sampleRate){
       val = strength.size()>0?strength[0].size()>0?strength[0][0]:-1:-1;
        if(val>0){
            val*=NOVELTY_MULT;
            frameCount = 0;
        }
    }

    
    
    return val;
        


}

void EssentiaOnset::setHopSize(int hS){
    
    hopSize = hS;
    fc->configure("hopSize",hopSize);
    superFluxP->configure("frameRate",sampleRate*1.0/hopSize);
    
}

void EssentiaOnset::preprocessPool(){
    
    pool.set("i.centroid",pool.value<Real>("i.centroid")*sampleRate/2);
    vector<Real> v = pool.value<vector<Real> >("i.mfcc");
    for(auto& e:v){
        e/=(frameSize/2 +1);
    }
    pool.set("i.mfcc", v);
    
    
}