#include "main.h"


void essentiaRT::setup(t_classid c)
{
    FLEXT_CADDMETHOD_(c, 0, "features", m_features);
    FLEXT_CADDMETHOD_(c, 0, "settings", m_settings);
    FLEXT_CADDMETHOD_(c,0,"delayMode",m_delayMode);
    FLEXT_CADDMETHOD_(c,0,"threshold",m_threshold);
    essentia::init();
    cout <<"arch : " << 8* sizeof(t_sample) << endl;
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


    audioBuffer = vector<Real> (frameSize,0);
    audioBufferOut= vector<Real> (frameSize,0);


    essentiaBufferCounter = 0;


    if(argc==1) onset_thresh = flext::GetAFloat(argv[0]);
    
    
    onsetDetection = new EssentiaOnset(frameSize, hopSize, sampleRate,pool,onset_thresh);
    SFX = new EssentiaSFX(frameSize,512,sampleRate);

    isSFX = false;
    isAggregating = false;


    
    //m_features(argc, argv);
}


essentiaRT::~essentiaRT()
{
    cout << SFX <<"."<< onsetDetection << endl;
    delete SFX;
    delete onsetDetection;
    
}

void essentiaRT::CbSignal()//m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs)
{
    int n = Blocksize();
    
    const t_sample *in = InSig()[0];//insigs[0];
    
    audioBuffer.resize(n);
    memcpy(&audioBuffer[0], in, n*sizeof(t_sample));

    
    Real onset = onsetDetection->compute(audioBuffer, audioBufferOut);
    if(onset>0){
        vector<Real> tst(1,onset) ;
        pool.set("i.strength",tst);
        onsetCB();
    }
    if(isSFX && !isAggregating){
     SFX->compute(audioBuffer);
    }
    
    cout << audioBuffer << endl;

}

void essentiaRT::onsetCB(){
    //trigger sfx
    //ioi mode: output last,clear,then retrigger sfx
    if(delayMode ==0 ){
        try{
            // careful with that : aggregate time should be inferior than 2 consecutive callbacks for non blocking audio thread
            if(aggrThread.joinable()){
            aggrThread.join();
            }
            aggrThread.~thread();
        aggrThread = thread(&essentiaRT::m_sfxAggr,this,nullptr);
        }
        catch(exception const& e){
            std::cout << e.what() << std::endl;
        }
        SFXTimer.Delay(MAX_SFX_TIME);
        isSFX=true;
        
    }
    // delay mode clear, trigger sfx and start timer
    else if(!isAggregating){
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

void essentiaRT::m_sfxAggr(void * i ){
    isAggregating = true;
    SFX->aggregate();
    outputListOfFeatures(getFeatures(SFX->aggrPool),2);
    SFX->clear();
    isAggregating = false;
    
    
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


void essentiaRT::m_threshold(float thresh){
    onsetDetection->superFluxP->configure("threshold",thresh);
    
}





