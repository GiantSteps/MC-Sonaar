#include "main.h"
#include "essentia/essentia.h"


bool essentiaRT2::inited = false;


void parseArgs(int argc,const t_atom *argv);




void essentiaRT2::setup(t_classid c)
{
    essentia::init();
    post("setup");
    
}

essentiaRT2::essentiaRT2 (int argc,const t_atom *argv)
{
    
    // Flext
    
    FLEXT_ADDBANG(0,my_bang);
    FLEXT_ADDTIMER(OutTimer,outputIt);
    
    post("creating");
    /////// PARAMS //////////////
    sampleRate = Samplerate();
    fR=1024;
    hop=1024;

    audioBufferCounter = 0;
    outRate = -1;
    
    
    
    parseArgs(argc,argv);
    audioBuffer = vector<Real> (fR,0.0);
    buildAlgo();
    
}

essentiaRT2::~essentiaRT2()
{
    
    
}

void essentiaRT2::m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs)
{
    for(int i = inputVectors.size()-1 ; i >=0  ; i--){
        const t_sample *in = insigs[i];
        
        while(n--) {
            //Fill Essentia vector every hopsize
            audioBuffer[audioBufferCounter] = *(in++);
            
            if(audioBufferCounter%hop == 0){
                int splitingPoint = (audioBufferCounter+1)%fR;
                int splitToEnd = fR - splitingPoint;
                memcpy(&inputVectors[i][0], &audioBuffer[splitingPoint], splitToEnd*sizeof(Real));
                memcpy(&inputVectors[i][splitToEnd], &audioBuffer[0], splitingPoint*sizeof(Real));
                if(i==0){
                    compute();
                    if(outRate==-1){
                        outputIt(NULL);
                    }
                }
            }
            audioBufferCounter++;
            audioBufferCounter%=fR;
            
        }
    }
    
    
}





void essentiaRT2::my_bang() {
    outputIt(NULL);
}

void essentiaRT2::outputIt(void *){
    int i = 0;
    for(auto out:outputStruct){
        if(out.isVector){
            ToOutList(i,Helper::floatVectorToList(outputStruct[i].aggregateVector()));
        }
        else{
            ToOutFloat(i, outputStruct[i].aggregateReal());
            
        }
        outputStruct[i].resetCounter();
        
        i++;
    }
    
};
void essentiaRT2::buildAlgo(){
    
    myAlgo = essentia::standard::AlgorithmFactory::create(name);
    post("building Algo");
    vector<string> paramNames = myAlgo->defaultParameters().keys();
    
    if (std::find(paramNames.begin(), paramNames.end(), "frameRate") != paramNames.end()){
        myAlgo->configure("frameRate",sampleRate*1.0/hop);
    }
    if (std::find(paramNames.begin(), paramNames.end(), "sampleRate") != paramNames.end()){
        myAlgo->configure("sampleRate",sampleRate);
    }
    if (std::find(paramNames.begin(), paramNames.end(), "hopSize") != paramNames.end()){
        myAlgo->configure("hopSize",hop);
    }
    if (std::find(paramNames.begin(), paramNames.end(), "frameSize") != paramNames.end()){
        myAlgo->configure("frameSize",fR);
    }
    
    
    
    for(auto a:paramsS){
        myAlgo->configure(a.first,a.second);
    }
    for(auto a:paramsF){
        myAlgo->configure(a.first,a.second);
    }
    post(" ");
    post("inputs:");
    int inIdx = 0;
    inputVectors.resize(myAlgo->inputs().size());
    for(auto in:myAlgo->inputs()){
        
        post(in.first.c_str());
        post(nameOfType(in.second->typeInfo()).c_str());
        if(in.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            inputVectors[inIdx]  = vector<Real>(fR,0);
            AddInSignal(in.first.c_str());
            in.second->set(inputVectors[inIdx]);
        }
        else if(in.second->typeInfo() ==typeid(essentia::Real)){
            error("Real input notAllowed ignoring input : ");
            error(in.first.c_str());
        }
        inIdx++;
    }
    post(" ");
    post("outputs:");
    outputStruct.resize(myAlgo->outputs().size());
    int outI = 0;
    for(auto out:myAlgo->outputs()){
        
        
        post(nameOfType(out.second->typeInfo()).c_str());
        if(out.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            outputStruct[outI] = ioStruct(out.first,true);
            AddOutList(out.first.c_str());
            out.second->set(outputStruct[outI].getNextVectorValue());
        }
        else if(out.second->typeInfo() ==typeid( essentia::Real)){
            outputStruct[outI] = ioStruct(out.first,false);
            AddOutFloat(out.first.c_str());
            out.second->set(outputStruct[outI].getNextRealValue());
        }
        else{
            post("cant parse outlet of type : " );post( essentia::nameOfType(out.second->typeInfo()).c_str());
            
        }
        post(out.first.c_str());
        outI++;
        
    }
    
    
    if(outRate>0){
        OutTimer.Periodic(outRate);
    }
    
}


bool essentiaRT2::compute(){
    
    for(int i = 0 ; i < outputStruct.size() ; i++){
        if(outputStruct[i].isVector){
            myAlgo->output(outputStruct[i].name).set(outputStruct[i].getNextVectorValue());
        }
        else{
            myAlgo->output(outputStruct[i].name).set(outputStruct[i].getNextRealValue());
        }
    }
    
    myAlgo->compute();
    
    
    
    
}


void essentiaRT2::parseArgs(int argc, const t_atom *argv){
    if(argc == 0){post ( "no argument provided" );return;}
    int argIdx = 0;
    
    while (argIdx < argc &&IsFloat(argv[argIdx]) && !IsString(argv[argIdx])) {
        
        switch (argIdx) {
            case 0:
                fR = GetFloat(argv[argIdx]);
                break;
            case 1:
                hop = GetFloat(argv[argIdx]);
                
            default:
                break;
        }
        argIdx++;
        
    }
    if(!IsString(argv[argIdx])){
        post("no valid name found");
    }
    else{
        name = GetString(argv[argIdx]);
        argIdx++;
    }
    
    if(IsFloat(argv[argIdx])){
        outRate =GetFloat(argv[argIdx])/1000.0;
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
            if(IsString(argv[argIdx])){paramsS[curName] = GetString(argv[argIdx]);}
            else if(IsFloat(argv[argIdx])){paramsF[curName] = GetFloat(argv[argIdx]);}
        }
        argIdx++;
        
    }
    
    
    
    
}




