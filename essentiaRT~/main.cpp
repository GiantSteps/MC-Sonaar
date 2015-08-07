#include "main.h"


void essentiaRT::setup(t_classid c)
{
    FLEXT_CADDMETHOD_(c, 0, "outInstant", onsetCB);
    FLEXT_CADDMETHOD_(c,0,"delayMode",m_delayMode);
    FLEXT_CADDMETHOD_(c,0,"threshold",m_threshold);
    FLEXT_CADDMETHOD_(c,0,"rthreshold",m_rthreshold);
    FLEXT_CADDMETHOD_(c,0,"combine",m_combine);
    essentia::init();
#ifdef _DEBUG
    cout <<"arch : " << 8* sizeof(t_sample) << endl;
#endif
    //essentia::setDebugLevel(essentia::EAll);
    
}

essentiaRT::essentiaRT (int argc,const t_atom *argv):
sampleRate(Samplerate()),
frameSize(FRAMESIZE),
hopSize(256),
onset_thresh(1.25),
delayMode(0)


{
    
    
    // Flext
    AddInSignal("In");
    AddOutSignal("Out");
    AddOutList("Onsets");
    AddOutList("OnsetInfo");
    
    FLEXT_ADDBANG(0,my_bang);
    FLEXT_ADDTIMER(SFXTimer, m_sfxAggr);
    
    
    audioBuffer = vector<Real> (hopSize,0);
    audioBufferOut= vector<Real> (hopSize,0);
    
    
    essentiaBufferCounter = 0;
    
    
    if(argc==1) onset_thresh = flext::GetAFloat(argv[0]);
    
    onsetDetection.setup(frameSize, hopSize, sampleRate,onset_thresh);
    SFX.setup(2048,256,sampleRate);
    
    isComputingSFX = false;
    isAggregatingSFX = false;
    
    
}


essentiaRT::~essentiaRT()
{
    if(aggrThread.joinable()){
        aggrThread.join();
    }
    
    
}

void essentiaRT::CbSignal()
{
    int n = checkBlockSize();
    memcpy(&audioBuffer[0], InSig()[0], n*sizeof(t_sample));
    compute();
    
    
}

// 64 bit : copy double to array of float
void essentiaRT::CbSignal64(){
    int n = checkBlockSize();
    while(n--){audioBuffer[n] = InSig()[0][n];}
    compute();
}


void essentiaRT::compute(){
    
    
    Real onset = onsetDetection.compute(audioBuffer, audioBufferOut);
    
    
    // on local maxima
    if(onset>0){
        
        vector<Real> tst(1,onset) ;
        onsetDetection.pool.set("i.strength",tst);
        onsetCB();
        
        
        
    }
    
    // on activation slope
    else if (onset == 0){
        ToOutBang(1);
        // we aggregate as soon as possible to avoid next transient noise
        if(delayMode ==0 ){
            m_sfxAggr(nullptr);
        }
        
    }
    
    
    
    // if compute SFX
    if(isComputingSFX && !isAggregatingSFX){
        SFX.compute(audioBuffer);
    }
    sfxCount= sfxCount>=0?MAX(0,sfxCount-Blocksize()):-1;
    if(sfxCount == 0 ){
        m_sfxAggr(nullptr);
        sfxCount = -1;
    }
    
    
}

void essentiaRT::onsetCB(){
    
    //trigger sfx
    //ioi mode: output last,clear,then retrigger sfx
    if(delayMode ==0 ){
        sfxCount = MAX_SFX_TIME*sampleRate;
        isComputingSFX=true;
        
    }
    // delay mode clear, trigger sfx and start timer
    else if(!isAggregatingSFX){
        SFX.clear();
        isComputingSFX=true;
        sfxCount = delayMode/1000.*sampleRate;
        
    }
    else{
        isComputingSFX=true;
        sfxCount = delayMode/1000.*sampleRate;
    }
    
    
    
    onsetDetection.preprocessPool();
    //output onsetStrength first
    std::map<string, vector<Real> > features = getFeatures(onsetDetection.pool);
    std::map<string, vector<Real>  >::iterator st = features.find("i.strength");
    if(st!= features.end()){
        AtomList listOut(2);
        SetString(listOut[0],"i.strength");
        SetFloat(listOut[1],(st->second)[0]);
        ToOutList(1, listOut);
        features.erase(st);
    }
    
    
    //output the rest
    outputListOfFeatures(features);
    onsetDetection.pool.clear();
    
    
    
    
}

int essentiaRT::checkBlockSize(){
    if(hopSize!=Blocksize()){
        //        onsetDetection.setHopSize(hopSize);
        hopSize = Blocksize();
        audioBuffer.resize(hopSize);
    }
    return hopSize;
}




void essentiaRT::my_bang() {
    std::map<string, vector<Real> > features = getFeatures(onsetDetection.pool);
    outputListOfFeatures(features);
}

void essentiaRT::m_sfxAggr(void * i ){
#ifdef THREADED_SFX
    try{
        // careful with that : aggregate time should be inferior than 2 consecutive callbacks for non blocking audio thread
        if(aggrThread.joinable()){aggrThread.join();}
        aggrThread.~thread();
        isAggregatingSFX = true;
        SFX.preprocessPool();
        aggrThread = thread(&essentiaRT::aggrThreadFunc,this);
        
    }
    catch(exception const& e){std::cout << e.what() << std::endl;}
#else
    isAggregatingSFX = true;
    SFX.preprocessPool();
    aggrThreadFunc();
#endif
    
}


void essentiaRT::aggrThreadFunc(){
    if(SFX.sfxPool.getRealPool().size()>0){
        isAggregatingSFX = true;
        SFX.aggregate();
        outputListOfFeatures(getFeatures(SFX.aggrPool),2);
        SFX.clear();
        
    }
    isAggregatingSFX = false;
}


std::map<string, vector<Real> > essentiaRT::getFeatures(const Pool & p)
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
    // safety for aggregating on more than 1 frame aquired
    if(del == 0){
        delayMode = 0;
        return;}
    
    int minTime =(1.1*SFX.frameSize)*1000.0/ sampleRate;
    if(del<minTime){
        delayMode=minTime;
        post("can't set a delay < %i ms",minTime);
    }
    else if(del> MAX_SFX_TIME*1000){
        post("can't set a delay > %i ms",MAX_SFX_TIME*1000);
        delayMode = MAX_SFX_TIME*1000;
    }
    else{
        delayMode = del;
    }
}


void essentiaRT::m_threshold(float thresh){
    if(thresh<0)
        thresh = 0;
    onsetDetection.superFluxP->configure("threshold",MAX(0,thresh*1.0/NOVELTY_MULT));
}


void essentiaRT::m_rthreshold(float rthresh){
    if(rthresh<1)
        rthresh = 0;
    
    onsetDetection.superFluxP->configure("ratioThreshold",MAX(0,rthresh));
    
}
void essentiaRT::m_combine(float c){
    onsetDetection.combineMs = c;
}


