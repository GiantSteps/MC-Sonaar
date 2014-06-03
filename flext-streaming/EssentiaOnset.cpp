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
    
    
    AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();
    
    fc = factory.create("FrameCutter",
                        "frameSize",frameSize,
                        "hopSize",hopSize,
                        "startFromZero" , true,
                        "validFrameThresholdRatio", 1,
                        "lastFrameToEndOfFile",true,
                        "silentFrames","keep"
                        );
    

    
    w = factory.create("Windowing","type","hann","Normalize",false,"zeroPhase",false);
    
    spectrum = factory.create("Spectrum");
    triF = factory.create("Triangularbands","Log",true);
    
    superFluxF = factory.create("SuperFluxNovelty","Online",true);
    superFluxP= factory.create("SuperFluxPeaks","rawmode" , true,"threshold" ,threshold,"startFromZero",false,"frameRate", sampleRate*1.0/hopSize,"combine",50);
    
    
    centroidF = factory.create("Centroid");
    
    mfccF = factory.create("MFCC","inputSize",frameSize/2);

    
    
    
    gen = factory.create("RingBufferInput","bufferSize",frameSize*2,"blockSize",hopSize);
    
    // buffer for getting the onset back
    essout = (streaming::RingBufferOutput*)factory.create("RingBufferOutput","bufferSize",hopSize,"blockSize",(int)1);
    // Audio Rate output, ATM , SuperFlux Novelty function
    DBGOUT = (streaming::RingBufferOutput*)factory.create("RingBufferOutput","bufferSize",hopSize,"blockSize",(int)1);
    
    // cutting, overlapping
    gen->output("signal") >> fc->input("signal");
    fc->output("frame") >> w->input("frame");
    w->output("frame") >> spectrum->input("frame");
    
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
    
    
    //Audio out
    superFluxF->output("Differences")  >>  DBGOUT->input("signal");
   
    
    //2 Pool
    connectSingleValue(centroidF->output("centroid"),poolin,"inst.centroid");
    connectSingleValue(mfccF->output("mfcc"),poolin,"inst.mfcc");

    //connectSingleValue(triF->output("bands"),poolin,"inst.tri");

    
    
    
    network = new scheduler::Network(gen);
    network->init();

}


float EssentiaOnset::compute(vector<Real>& audioFrameIn, vector<Real>& output){
    ((essentia::streaming::RingBufferInput*)gen)->add(&audioFrameIn[0],audioFrameIn.size());
    gen->process();
    
    network->runStack();
    
    output.resize(audioFrameIn.size());
    int retrievedSize = DBGOUT->get(&output[0], 1);
    Real audioout = retrievedSize>0?output[retrievedSize-1] : 0;
    

    
    
    retrievedSize = essout->get(&output[0], 1);
    Real val = retrievedSize>0?output[retrievedSize-1] : 0;

    for (int i =0; i< output.size() ; i++){
        output[i]=audioout;
    }
    
    return val;
        


}