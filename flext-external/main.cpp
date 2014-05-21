#include "main.h"


void pd_essentia::setup(t_classid c)
{
    FLEXT_CADDMETHOD_(c, 0, "features", m_features);
}

pd_essentia::pd_essentia(int argc,const t_atom *argv)
{
    AddInSignal("In");
    AddOutSignal("Out");
    AddOutList("FFT");
    
    FLEXT_ADDBANG(0,my_bang);

    
    /////// PARAMS //////////////
    int sampleRate = Samplerate();
    int frameSize = 1024;
    int hopSize = 512;
    
    essentia.setup(sampleRate, frameSize,hopSize);
    
    for(int i=0; i<frameSize; i++)
        audioBuffer.push_back(0.0);

    essentiaBufferCounter = 0;
    
    m_features(argc, argv);
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
//            std::map<string, vector<Real> > features = essentia.getFeatures();
//            outputListOfFeatures(features);
        }
        *(out++) = *(in++);
    }
}

void pd_essentia::m_features(int argc, const t_atom *argv)
{
    essentia.currentAlgorithms.clear();
    for(int i=0; i<argc; i++)
        essentia.currentAlgorithms[GetString(argv[i])] = true;
    
    //Always want onsets
    essentia.currentAlgorithms["onsets"] = true;
        
}

void pd_essentia::my_bang() {
    std::map<string, vector<Real> > features = essentia.getFeatures();
    outputListOfFeatures(features);
}

void pd_essentia::outputListOfFeatures(const std::map<string, vector<Real> >& features)
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
    

    



