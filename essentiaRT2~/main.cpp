#include "main.h"

void parseArgs(int argc,const t_atom *argv);
void essentiaRT2::setup(t_classid c)
{
    
    
}

essentiaRT2::essentiaRT2 (int argc,const t_atom *argv)
{
    parseArgs(argc,argv);
    buildAlgo();
    // Flext
    
    FLEXT_ADDBANG(0,my_bang);
    FLEXT_ADDTIMER(OutTimer,outputIt);
    
    
    /////// PARAMS //////////////
    sampleRate = Samplerate();
    frameSize = 2048;
    hopSize = 256;
    
    blockCountMax = (int)(hopSize/Blocksize());
    
    
    
    for(int i=0; i<hopSize; i++){
        audioBuffer.push_back(0.0);
        audioBufferOut.push_back(0.0);
    }
    
    essentiaBufferCounter = 0;
    
    
    //m_features(argc, argv);
    
}

essentiaRT2::~essentiaRT2()
{
    essentia::shutdown();
    
}

void essentiaRT2::m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs)
{
    const t_sample *in = insigs[0];
    t_sample *out = outsigs[0];
    while(n--) {
        //Fill Essentia vector every hopsize , buffering is handled inside streaming algorithms
        audioBuffer[essentiaBufferCounter] = *(in++);
        // trick to get onset or novelty function at signal rate on outlet1
        *(out++) = audioBufferOut[essentiaBufferCounter];
        essentiaBufferCounter++;
        
    }
    
    blockCount++;
    if(blockCount>=blockCountMax) {
        
    }
}





void essentiaRT2::my_bang() {
    outputIt(NULL);
}

void essentiaRT2::outputIt(void *){
    post("outPool");
    int idx =0 ;
    for(auto out:myAlgo->outputs()){
        if(out.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            AtomList list(pool.value<std::vector<Real> >(out.first).size());
            int i = 0;
            for(auto t:pool.value<std::vector<Real> >(out.first)){
                SetFloat(list[i],t);
                i++;
            }
            ToOutList(idx, list);
        }
        else if(out.second->typeInfo() ==typeid(essentia::Real)){
            ToOutFloat(idx, pool.value<Real>(out.first));
        }
        idx++;
    }
    
};
void essentiaRT2::buildAlgo(){
    essentia::init();
    myAlgo = essentia::streaming::AlgorithmFactory::create(curAlgo.name);
    post("building Algo");
    for(auto a:curAlgo.paramsS){
        myAlgo->configure(a.first,a.second);
    }
    for(auto a:curAlgo.paramsF){
        myAlgo->configure(a.first,a.second);
    }
    post(" ");
    post("inputs:");
    for(auto in:myAlgo->inputs()){
        post(in.first.c_str());
        post(nameOfType(in.second->typeInfo()).c_str());
        
//        thisClassId()
        AddMethod(thisClassId(),i,<#bool (*m)(flext_base *)#>);
        if(in.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            AddInList(in.first.c_str());
        }
        else if(in.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            AddInFloat(in.first.c_str());
        }
    }
    post(" ");
    post("outputs:");
    for(auto out:myAlgo->outputs()){
        post(out.first.c_str());
        post(nameOfType(out.second->typeInfo()).c_str());
        *out.second >> PC(pool,out.first);
        AddOutFloat(out.first.c_str());
    }
    
    
    if(curAlgo.outRate!=0){
        OutTimer.Periodic(curAlgo.outRate);
    }
}

void essentiaRT2::parseArgs(int argc, const t_atom *argv){
    if(argc == 0){post ( "no argument provided" );return;}
    int argIdx = 0;
    
    while (argIdx < argc &&IsFloat(argv[argIdx]) && !IsString(argv[argIdx])) {
        
        switch (argIdx) {
            case 0:
                curAlgo.fR = GetFloat(argv[argIdx]);
                break;
            case 1:
                curAlgo.hop = GetFloat(argv[argIdx]);
                
            default:
                break;
        }
        argIdx++;
        
    }
    if(!IsString(argv[argIdx])){
        post("no valid name found");
    }
    else{
        curAlgo.name = GetString(argv[argIdx]);
        argIdx++;
    }
    
    if(IsFloat(argv[argIdx])){
        curAlgo.outRate =GetFloat(argv[argIdx])/1000.0;
        argIdx++;
    }
    
    
    int beginMsg = argIdx;
    string curName = "";
    while(argIdx < argc){
        if((argIdx - beginMsg)%2==0){
            if(!IsString(argv[argIdx])){post("no valid parameter : [name value] list");}
            else{ curName = GetAString(argv[argIdx]);}
        }
        else{
            if(IsString(argv[argIdx])){curAlgo.paramsS[curName] = GetString(argv[argIdx]);}
            else if(IsFloat(argv[argIdx])){curAlgo.paramsF[curName] = GetFloat(argv[argIdx]);}
        }
        argIdx++;
        
    }
    
    
    
    
}




