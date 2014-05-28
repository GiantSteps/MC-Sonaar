#include "main.h"


void essentiaRT::setup(t_classid c)
{
    FLEXT_CADDMETHOD_(c, 0, "features", m_features);
    FLEXT_CADDMETHOD_(c, 0, "settings", m_settings);
    
    
    
    //essentia::setDebugLevel(essentia::EAll);

}

essentiaRT::essentiaRT (int argc,const t_atom *argv): onset_thresh(1.25)
{
    // Flext
    AddInSignal("In");
    AddOutSignal("Out");
    AddOutList("Onsets");
    AddOutList("OnsetInfo");
    
    FLEXT_ADDBANG(0,my_bang);
    FLEXT_ADDTIMER(SFXTimer, m_sfxAggr);
    
    /////// PARAMS //////////////
     sampleRate = Samplerate();
     frameSize = 2048;
     hopSize = 256;
    
    
    
    framecount =0;
    
    for(int i=0; i<hopSize; i++){
        audioBuffer.push_back(0.0);
        audioBufferOut.push_back(0.0);
    }

    essentiaBufferCounter = 0;

    
    
    // Declare Essentia Components
    essentia::init();
    
    if(argc==1) onset_thresh = Helper::getReal(argv[0]);
    
    onsetDetection.setup(frameSize, hopSize, sampleRate,&pool,onset_thresh);
    
    SFX.setup(frameSize, hopSize, sampleRate, &pool);
    SFXLength = 0.1;
    isSFX = false;


    
    //m_features(argc, argv);
}


essentiaRT::~essentiaRT()
{
    essentia::shutdown();
    
}

void essentiaRT::m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs)
{
    const t_sample *in = insigs[0];
    t_sample *out = outsigs[0];
    while(n--) {
        //Fill Essentia vector
        audioBuffer[essentiaBufferCounter] = *in;
        
        essentiaBufferCounter++;
        if(essentiaBufferCounter>=hopSize) {
            essentiaBufferCounter=0;
            
            Real onset = onsetDetection.compute(audioBuffer, audioBufferOut);
            
            if(isSFX)SFX.compute(audioBuffer);
            
            vector<Real> tst(1,onset) ;
            pool.set("onset_strength",tst);
            
            if(onset>0)onsetCB();
            
            framecount++;


        }
        // trick to get onset at signal rate on outlet1
        *(out++) = audioBufferOut[essentiaBufferCounter];
    }
}

void essentiaRT::onsetCB(){
    if(!isSFX){
        isSFX=true;
        SFXTimer.Delay(SFXLength);
        
    }
    my_bang();
}

void essentiaRT::m_features(int argc, const t_atom *argv)
{
    currentAlgorithms.clear();
    for(int i=0; i<argc; i++)
        currentAlgorithms[GetString(argv[i])] = true;
    
    //Always want onsets
    currentAlgorithms["onsets"] = true;
}


// Not Working
void essentiaRT::m_settings(int argc, const t_atom *argv)
{
    essentia::ParameterMap pM;
       string curset =  GetString(argv[0]);
    
    if(curset=="onset_threshold"){
        pM.add("threshold",Helper::getReal(argv[1]));
        onsetDetection.superFluxP->setParameters(pM);

    }

}

void essentiaRT::my_bang() {
    std::map<string, vector<Real> > features = getFeatures(pool);
    outputListOfFeatures(features);
}

void essentiaRT::m_sfxAggr(void *){
    SFX.aggregate();
    outputListOfFeatures(getFeatures(SFX.aggrPool),2);
    isSFX = false;
    SFX.clear();
    
}


std::map<string, vector<Real> > essentiaRT::getFeatures(Pool p)
{
 
        std::map<string, vector<vector<Real> > >  vectorsIn =     p.getVectorRealPool();
    std::map<string, vector<Real> > vectorsOut;
    
    for(std::map<string, vector<vector<Real> > >::iterator iter = vectorsIn.begin(); iter != vectorsIn.end(); ++iter)
    {
        string k =  iter->first;
        vector<Real> v = (iter->second)[0];
        
        vectorsOut[k] = v;
    }
    
    std::map<string, vector<Real > >  realsIn =  p.getSingleVectorRealPool();
    
    for(std::map<string, vector<Real > >::iterator iter = realsIn.begin(); iter != realsIn.end(); ++iter)
    {
        string k =  iter->first;
        vector<Real> v = iter->second;
        
        vectorsOut[k] = v;
    }
    std::map<string, Real  >  reals2In =  p.getSingleRealPool();
    
    for(std::map<string, Real  >::iterator iter = reals2In.begin(); iter != reals2In.end(); ++iter)
    {
        string k =  iter->first;
        vector<Real> v = vector<Real>(1,iter->second);
        
        vectorsOut[k] = v;
    }

    
    return vectorsOut;
}


void essentiaRT::outputListOfFeatures(const std::map<string, vector<Real> >& features,int outlet)
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
        
       ToQueueList(outlet, listOut);
    }
}
    





