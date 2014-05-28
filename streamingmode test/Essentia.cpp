//
//  Essentia.cpp
//  pd_essentia~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//

#include "Essentia.h"


using namespace essentia;
using namespace essentia::streaming;
Essentia::Essentia()
{
    

}
Essentia::~Essentia()
{
    
    //        delete audio;
    delete network;


    
    essentia::shutdown();
}

void Essentia::setup(int sampleRate, int frameSize, int hopSize)
{
    
    essentia::init();
//    essentia::setDebugLevel(EAll);

    
    
    this->sampleRate = sampleRate;
    this->frameSize = frameSize;
    this->hopSize = hopSize;
//    


    
    AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();


//    
//    input = factory.create("RingBufferInput");
    fc = factory.create("FrameCutter",
                        "frameSize",frameSize,
                        "hopSize",hopSize,
                        "startFromZero" , true,
                        "validFrameThresholdRatio", 1,
                        "lastFrameToEndOfFile",true);
    
    w = factory.create("Windowing");

    spectrum = factory.create("Spectrum");
    triF = factory.create("Triangularbands","Log",false);

    superFluxF = factory.create("SuperFluxNovelty","Online",true);
    superFluxP= factory.create("SuperFluxPeaks","rawmode" , true,"threshold" ,.8,"startFromZero",false);



//    
//    input.add
//    
//    input->output("signal") >> Avg->input("signal");
//    runGenerator(input);
    
    
    
    gen = factory.create("RingBufferInput","bufferSize",frameSize*10,"blockSize",hopSize);

    essout = (streaming::RingBufferOutput*)factory.create("RingBufferOutput","bufferSize",hopSize*10,"blockSize",(int)1);
    
    

    gen->output("signal") >> fc->input("signal");
    fc->output("frame") >> w->input("frame");
    
    w->output("frame") >> spectrum->input("frame");
    spectrum->output("spectrum") >> triF->input("spectrum");
    triF->output("bands")>>superFluxF->input("bands");
    
    superFluxF->output("Differences")  >>    superFluxP->input("novelty");
    superFluxP->output("peaks") >>  essout->input("signal");
    
   // connectSingleValue(Avg->output("signal"), pool,"avg");
    
//    
    
    
    
    
    network = new scheduler::Network(gen);
    network->init();
//    vector<Real> audioFrame(2048);
//    gen->setVector(&audioFrame);
//    gen->process();
//    gen->process();
//    gen->process();
//    gen->process();
    framecount =0;
   
}

void Essentia::compute(vector<Real>& audioFrame,vector<Real>& output)
{
//    fc->input("signal").set(audioFrame);
   // pool.clear();

    
    ((essentia::streaming::RingBufferInput*)gen)->add(&audioFrame[0],audioFrame.size());
    gen->process();

    network->runStack();

    output.resize(audioFrame.size());
    int retrievedSize = essout->get(&output[0], output.size());
    if(retrievedSize==0){
        for (int i =0; i< audioFrame.size() ; i++){
            output[i]=0;
        }
        //cout<< "no ringbufferOutput" <<endl;
    }
    else{
//        Real difff = output[retrievedSize-1];
//        
//        for (int i =retrievedSize; i< audioFrame.size() ; i++){
//            output[i]=difff;
//            }
//        cout << "retrieved "<< retrievedSize << " frame " << framecount << endl;
//        for (int i =0 ; i < retrievedSize;i++){
            if (output[retrievedSize-1]>0){
                                cout << "Boom" << framecount << endl;
                                
            }
//        }

    }
    

    

    framecount++;
 

    

    

    
    
    
    
    //IF WE GOT AN ONSET -> OUTPUT FEATURES
    

    
}

//vector<flext::AtomList> Essentia::getFeatures()
//{
//    
//
//}

std::map<string, vector<Real> > Essentia::getFeatures()
{
    std::map<string, vector<Real > >  vectorsOut =     pool.getRealPool();
//    std::map<string, vector<Real> > vectorsOut;
//    
//    for(std::map<string, vector<Real > >::iterator iter = vectorsIn.begin(); iter != vectorsIn.end(); ++iter)
//    {
//        string k =  iter->first;
//        vector<Real> v = (iter->second)[0];
//        
//        vectorsOut[k] = v;
//    }
    return vectorsOut;
}