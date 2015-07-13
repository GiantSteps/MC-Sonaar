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
    this->frameSize = FRAMESIZE;
    this->hopSize = hS;
    
    
    combineMs = 150;
    strength.resize(2);
    
    AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();
    
    fc = factory.create("FrameCutter",
                        "frameSize",frameSize,
                        "hopSize",hopSize,
                        "startFromZero" , true,
                        "validFrameThresholdRatio", 1,
                        "lastFrameToEndOfFile",true,
                        "silentFrames","keep"
                        );
    

    
    w = factory.create("Windowing","type","hann"
                       );
    
    spectrum = factory.create("Spectrum");
    pspectrum = factory.create("PowerSpectrum");
    triF = factory.create("TriangularBands","log",false);
    
    superFluxF = factory.create("SuperFluxNovelty","binWidth",8,"frameWidth",2);
    superFluxP= factory.create("SuperFluxPeaks","ratioThreshold" , threshold,"threshold" ,0.5//,threshold/NOVELTY_MULT
                               ,"pre_max",30,"pre_avg",200,"frameRate", sampleRate*1.0/hopSize,"combine",combineMs);
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
    gen->output("signal") >> fc->input("signal");
    fc->output("frame")  >>  w->input("frame");
    w->output("frame") >> spectrum->input("frame");
    //w->output("frame") >> pspectrum->input("signal");
    // SuperFlux
    spectrum->output("spectrum") >> triF->input("spectrum");
    triF->output("bands")>>superFluxF->input("bands");
    superFluxF->output("Differences")  >>superFluxP->input("novelty");
    superFluxP->output("peaks") >> essout->input("data");
    
    // MFCC
    spectrum->output("spectrum") >> mfccF->input("spectrum");
    mfccF->output("bands") >> DEVNULL;
    
    
    // centroid
    spectrum->output("spectrum") >> centroidF->input("array");
    
    
    //Audio out & DBG
//    superFluxF->output("Differences")  >>  //DBGOUT->input("signal");
    //triF->output("bands") >> probe->input("data");
    //gen->output("signal")  >>  //DBGOUT->input("signal");
    
    //2 Pool
   connectSingleValue(centroidF->output("centroid"),pool,"i.centroid");
    connectSingleValue(mfccF->output("mfcc"),pool,"i.mfcc");

    //connectSingleValue(triF->output("bands"),poolin,"inst.tri");

    
    
    
    network = new scheduler::Network(gen);
    network->initStack();

//    essentia::setDebugLevel(essentia::EExecution | essentia::EScheduler);
}


float EssentiaOnset::compute(vector<Real>& audioFrameIn, vector<Real>& output){

    essout->setVector(&strength);

    if(audioFrameIn.size()!=gen->_bufferSize){
        cout << "resizing" <<audioFrameIn.size() << endl;
        gen->_bufferSize = audioFrameIn.size();
        gen->output(0).setAcquireSize(gen->_bufferSize);
        gen->output(0).setReleaseSize(gen->_bufferSize);
        gen->configure();
    }
    
    gen->add(&audioFrameIn[0], audioFrameIn.size());

    gen->process();
    
    
    
    network->runStack(false);
    dynamic_cast<AccumulatorAlgorithm * >(superFluxP)->finalProduce();
    network->runStack(false);


    int val =0;
    frameCount+= gen->_bufferSize;

    if(frameCount*1000.0>combineMs*sampleRate){
       val = strength.size()>0?strength[0].size()>0?1:0:0;
        if(val>0){
            frameCount = 0;

        }
    }

    strength.clear();


    
    return val;
        


}

void EssentiaOnset::preprocessPool(){
    
    pool.set("i.centroid",pool.value<Real>("i.centroid")*sampleRate/2);
    
}