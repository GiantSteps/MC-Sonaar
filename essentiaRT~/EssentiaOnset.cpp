//
//  EssentiaOnset.cpp
//  essentiaRT~
//
//  Created by martin hermant on 27/05/14.
//
//

#include "EssentiaOnset.h"



EssentiaOnset::~EssentiaOnset(){
    delete network;
    
}
EssentiaOnset::EssentiaOnset(){

}

void EssentiaOnset::setup(int fS,int hS,int sR,Pool& poolin,Real threshold){

    this->sampleRate = sR;
    this->frameSize = fS;
    this->hopSize = hS;
    this->pool = &poolin;
    
    
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
    

    
    w = factory.create("Windowing","type","hann","Normalize",true,"zeroPhase",true
                       );
    
    spectrum = factory.create("Spectrum");
    pspectrum = factory.create("PowerSpectrum");
    triF = factory.create("Triangularbands","Log",true);
    
    superFluxF = factory.create("SuperFluxNovelty","Online",true,"binWidth",3,"frameWidth",2);
    superFluxP= factory.create("SuperFluxPeaks","rawmode" , true,"threshold" ,threshold/NOVELTY_MULT,"startFromZero",false,"frameRate", sampleRate*1.0/hopSize,"combine",50);
    

    
    centroidF = factory.create("Centroid");
    
    mfccF = factory.create("MFCC","inputSize",frameSize/2);

    
    
    
    gen = factory.create("RingBufferInput","bufferSize",hopSize*2,"blockSize",hopSize);
    
    // buffer for getting the onset back
    essout = (streaming::RingBufferOutput*)factory.create("RingBufferOutput","bufferSize",2,"blockSize",(int)1);
    // Audio Rate output, ATM , SuperFlux Novelty function
    DBGOUT = (streaming::RingBufferOutput*)factory.create("RingBufferOutput","bufferSize",2,"blockSize",(int)1);
    
    probe = new streaming::VectorOutput< vector<Real> >();
    
    // cutting, overlapping
    gen->output("signal") >> fc->input("signal");
    fc->output("frame")  >>  w->input("frame");
    w->output("frame") >> spectrum->input("frame");
    //w->output("frame") >> pspectrum->input("signal");
    // SuperFlux
    spectrum->output("spectrum") >> triF->input("spectrum");
    triF->output("bands")>>superFluxF->input("bands");
    superFluxF->output("Differences")  >>superFluxP->input("novelty");
    superFluxP->output("peaks") >> essout->input("signal");
    
    // MFCC
    spectrum->output("spectrum") >> mfccF->input("spectrum");
    mfccF->output("bands") >> DEVNULL;
    
    
    // centroid
    spectrum->output("spectrum") >> centroidF->input("array");
    
    
    //Audio out & DBG
    superFluxF->output("Differences")  >>  DBGOUT->input("signal");
    //triF->output("bands") >> probe->input("data");
    //gen->output("signal")  >>  DBGOUT->input("signal");
    
    //2 Pool
   connectSingleValue(centroidF->output("centroid"),poolin,"i.centroid");
    connectSingleValue(mfccF->output("mfcc"),poolin,"i.mfcc");

    //connectSingleValue(triF->output("bands"),poolin,"inst.tri");

    
    
    
    network = new scheduler::Network(gen);
    network->init();

}


float EssentiaOnset::compute(vector<Real>& audioFrameIn, vector<Real>& output){
    ((essentia::streaming::RingBufferInput*)gen)->add(&audioFrameIn[0],audioFrameIn.size());
    gen->process();
    

    network->runStack();
    
    output.resize(audioFrameIn.size());
    int retrievedSize = DBGOUT->get(&output[0], output.size());
    Real audioout = retrievedSize>0?output[retrievedSize-1]*NOVELTY_MULT : 0;

    
    


    for (int i =retrievedSize-1; i< output.size() ; i++){
        output[i]=audioout;
    }

    
    
    retrievedSize = essout->get(&strength[0],1);
    Real val = retrievedSize>0?strength[retrievedSize-1]*NOVELTY_MULT : 0;


    
    return val;
        


}

void EssentiaOnset::preprocessPool(){
    
    pool->set("i.centroid",pool->value<Real>("i.centroid")*sampleRate/2);
    
}