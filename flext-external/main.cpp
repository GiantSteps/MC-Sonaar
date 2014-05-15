#include "main.h"


pd_essentia::pd_essentia()
{
    AddInSignal("In");
    AddOutSignal("Out");
    AddOutList("FFT");
    
    FLEXT_ADDBANG(0,my_bang);
    
    /////// PARAMS //////////////
    int sampleRate = Samplerate();
    int frameSize = 2048;
    int hopSize = 1024;
    
    essentia.setup(sampleRate, frameSize,hopSize);
    
    for(int i=0; i<frameSize; i++)
        audioBuffer.push_back(0.0);

    essentiaBufferCounter = 0;
}

pd_essentia::~pd_essentia()
{
    
}

void pd_essentia::m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs)
{
    const t_sample *in = insigs[0];
    t_sample *out = outsigs[0];
    while(n--) {
        //Fill Essentia vector
        audioBuffer[essentiaBufferCounter] = *in;
        
        essentiaBufferCounter++;
        if(essentiaBufferCounter>=essentia.frameSize) {
            essentiaBufferCounter=0;
            
            //Here we call the Essentia object for computation
            essentia.compute(audioBuffer);
            
            //Spit out the pool
            
//            vector<AtomList> features = essentia.getFeatures();
//            for(int i=0; i<features.size(); i++)
//                ToQueueList(1, features[i]);
        }
        *(out++) = *(in++);
    }
}





void pd_essentia::my_bang() {
    
}
    

    



