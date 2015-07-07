#include "main.h"


void essentiaRT::setup(t_classid c)
{
    FLEXT_CADDMETHOD_(c, 0, "features", m_features);
    FLEXT_CADDMETHOD_(c, 0, "settings", m_settings);
    FLEXT_CADDMETHOD_(c,0,"delayMode",m_delayMode);
    
    
    //essentia::setDebugLevel(essentia::EAll);

}

essentiaRT::essentiaRT (int argc,const t_atom *argv):
sampleRate(Samplerate()),
frameSize(FRAMESIZE),
hopSize(512),
onset_thresh(1.25)

{
    
    //Debug
//#ifdef DEBUG_PD
//    std::ofstream   fout("/dev/null");
//    std::cout.rdbuf(fout.rdbuf());
//#endif
    // Flext
    AddInSignal("In");
    AddOutSignal("Out");
    AddOutList("Onsets");
    AddOutList("OnsetInfo");
    
    FLEXT_ADDBANG(0,my_bang);
    FLEXT_ADDTIMER(SFXTimer, m_sfxAggr);

    
    /////// PARAMS //////////////
//     sampleRate = Samplerate();
//     frameSize = 2048;
//     hopSize = 256;

    blockCountMax = (int)(hopSize/Blocksize());


    audioBuffer = vector<Real> (frameSize,0);
    audioBufferOut= vector<Real> (frameSize,0);


    essentiaBufferCounter = 0;


    if(argc==1) onset_thresh = flext::GetAFloat(argv[0]);
    
    essentia::init();
    onsetDetection = new EssentiaOnset(frameSize, hopSize, sampleRate,pool,onset_thresh);
    SFX = new EssentiaSFX(frameSize,512,sampleRate);

    isSFX = false;


    
    //m_features(argc, argv);
}


essentiaRT::~essentiaRT()
{
    delete SFX;
    delete onsetDetection;
    essentia::shutdown();
    
}

void essentiaRT::m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs)
{
    const t_sample *in = insigs[0];
    t_sample *out = outsigs[0];
    while(n--) {
        //Fill Essentia vector every hopsize , buffering is handled inside streaming algorithms
        audioBuffer[essentiaBufferCounter] = *(in++);
        // trick to get onset or novelty function at signal rate on outlet1
        *(out++) = audioBufferOut[essentiaBufferCounter];
        essentiaBufferCounter++;

    essentiaBufferCounter=essentiaBufferCounter%frameSize;

    }
    blockCount=0;
    Real onset = onsetDetection->compute(audioBuffer, audioBufferOut);
    if(onset>0){
        vector<Real> tst(1,onset) ;
        pool.set("i.strength",tst);
        onsetCB();
    }
    if(isSFX)SFX->compute(audioBuffer);
    
    
//    blockCount++;
    
    

}

void essentiaRT::onsetCB(){
    //trigger sfx
    //ioi mode: output last,clear,then retrigger sfx
    if(delayMode ==0 ){
        m_sfxAggr(NULL);
        SFX->clear();
        SFXTimer.Delay(MAX_SFX_TIME);
        isSFX=true;
        
    }
    // delay mode clear, trigger sfx and start timer
    else if(!isSFX){
            SFX->clear();
            isSFX=true;
            SFXTimer.Delay(delayMode/1000.);
        
    }
    onsetDetection->preprocessPool();
    //output onsetStrength first
    std::map<string, vector<Real> > features = getFeatures(pool);
    std::map<string, vector<Real>  >::iterator st = features.find("i.strength");
    AtomList listOut(2);
    SetString(listOut[0],"i.strength");
    SetFloat(listOut[1],(st->second)[0]);
    ToOutList(1, listOut);
    features.erase(st);
    
    
    //output the rest
    outputListOfFeatures(features);
    


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
        pM.add("threshold",GetAFloat(argv[1]));
        onsetDetection->superFluxP->setParameters(pM);

    }

}

void essentiaRT::my_bang() {
    std::map<string, vector<Real> > features = getFeatures(pool);
    
    outputListOfFeatures(features);
}

void essentiaRT::m_sfxAggr(void *){

    SFX->aggregate();
    outputListOfFeatures(getFeatures(SFX->aggrPool),2);
    isSFX = false;
    
    
}


std::map<string, vector<Real> > essentiaRT::getFeatures(Pool p)
{
 
        std::map<string, vector<Real > >  vectorsIn =     p.getRealPool();
    std::map<string, vector<Real> > vectorsOut;
    
    for(std::map<string, vector<Real > >::iterator iter = vectorsIn.begin(); iter != vectorsIn.end(); ++iter)
    {
        string k =  iter->first;
        vector<Real> v = (iter->second);
        
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
        
       ToOutList(outlet, listOut);
    }
    
}


void essentiaRT::m_delayMode(int del){
    
    delayMode = del;
    //isSFX=false;
    
    
    
}
    





