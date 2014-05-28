#include "main.h"


superFlux::superFlux()
{
    AddInSignal("In");
    AddOutSignal("Out");
    AddOutList("FFT");
    
    FLEXT_ADDBANG(0,my_bang);
    
    /////// PARAMS //////////////
    int sampleRate = Samplerate();
    int frameSize = 2048;
    int hopSize = 256;
    
    essentia.setup(sampleRate, frameSize,hopSize);
    
    for(int i=0; i<hopSize; i++)
        audioBuffer.push_back(0.0);

    essentiaBufferCounter = 0;
//    TODO: check if we can get Pd block size
    audioOutput.resize(audioBuffer.size());
}

superFlux::~superFlux()
{
    
}

void superFlux::m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs)
{
    const t_sample *in = insigs[0];
    t_sample *out = outsigs[0];
    
    int blocksize = n;
    while(n--) {
        //Fill Essentia vector
        audioBuffer[essentiaBufferCounter] = *in;
        
        essentiaBufferCounter++;
        if(essentiaBufferCounter>=essentia.hopSize) {
            essentiaBufferCounter=0;
            // need an empty vector
            vector<Real> output_ess;
            //Here we call the Essentia object for computation
            essentia.compute(audioBuffer,output_ess);
            audioOutput = output_ess;
            //Spit out the pool
            //std::map<string, vector<Real> > features = essentia.getFeatures();
            //outputListOfFeatures(features);
        }
        in++;
        *(out++) = t_sample(audioOutput[essentiaBufferCounter]);
    }
}


void superFlux::my_bang() {
    
}

void superFlux::outputListOfFeatures(const std::map<string, vector<Real> >& features)
{
    for(std::map<string, vector<Real>  >::const_iterator iter = features.begin(); iter != features.end(); ++iter)
    {
        AtomList listOut(iter->second.size()+1);
        t_atom featureName;
        
        SetString(featureName, iter->first.c_str());
        
        listOut[0] = featureName;
        
        for(int i=0; i<iter->second.size(); i++) {
            t_atom featureValue;
        
            SetFloat(featureValue, iter->second[i]);
            
            listOut[i+1] = featureValue;
            
        }
        
        ToQueueList(1, listOut);
    }
}
    

    



