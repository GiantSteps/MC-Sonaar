#include "main.h"
#include "essentia/essentia.h"


bool essentiaRT2::debug = false;
short essentiaRT2::circularShift = 0b0100101;

void parseArgs(int argc,const t_atom *argv);




void essentiaRT2::setup(t_classid c)
{
    essentia::init();
    log("setup");

    
}

essentiaRT2::essentiaRT2 (int argc,const t_atom *argv)
{
    
    // Flext
    
    FLEXT_ADDBANG(0,my_bang);
    FLEXT_ADDMETHOD_(0, "postInfo", getAlgoDescription);
    FLEXT_ADDMETHOD_(0, "postAlgoList", getAllAlgos);
    FLEXT_ADDATTR_VAR1("debug", debug);
    
    log("creating");
    /////// PARAMS //////////////
    sampleRate = Samplerate();
    frameSize=Blocksize();
    hopSize=Blocksize();
    
    
    outSize = -1;
    outHopSize = -1;

    
    
    parseArgs(argc,argv);
    buildAlgo();
    
#ifdef FLEXT_TILDE
    audioBufferCounter = 0;
    audioBuffer = vector<Real> (frameSize,0.0);
#endif
    
    
    
}

essentiaRT2::~essentiaRT2()
{
    delete myAlgo;
    
}
#ifdef FLEXT_TILDE
void essentiaRT2::m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs)
{
    int my_n = n;
    if(isSpectrum){
        my_n/=2;
        my_n++;
        // be sure to synchronize ringbuffer
        audioBufferCounter = 0;
    }
    for(int i = inputVectors.size()-1 ; i >=0  ; i--){
        const t_sample *in = insigs[i];
        n = my_n;
        while(n--) {
            //Fill Essentia vector every hopSizesize
            audioBuffer[audioBufferCounter] = *(in++);
            
            audioBufferCounter++;
            audioBufferCounter%=(frameSize);
            if((audioBufferCounter)%(hopSize) == 0 ){
                int splitToEnd = frameSize - audioBufferCounter ;
                
                memcpy(&inputVectors[i][0], &audioBuffer[audioBufferCounter], splitToEnd*sizeof(Real));
                memcpy(&inputVectors[i][splitToEnd], &audioBuffer[0], audioBufferCounter*sizeof(Real));
                
                if(i==0){
                    hasInput = true;
                    compute();
                    if(outHopSize==-1){
                        outputIt(NULL);
                    }
                }
            }
            
            
            
        }
        
        
    }
    
    
    
    
}
#else
bool essentiaRT2::CbMethodResort(int inlet, const t_symbol *s, int argc, const t_atom *argv){
    
    if(inputStruct[inlet].type == ioStruct::REAL){
        if(argc>1){
            err("too much element on inlet  : "+std::to_string(inlet));
        }
        else{
            inputStruct[inlet].getNextRealValue() = GetAFloat(argv[0]);
            myAlgo->input(inputStruct[inlet].name).set(inputStruct[inlet].getRealValue());
        }
    }
    else if(inputStruct[inlet].type == ioStruct::VECTOR ){
        
        inputStruct[inlet].getNextVectorValue(argc) = Helper::listToVector(argc,argv);
        myAlgo->input(inputStruct[inlet].name).set(inputStruct[inlet].getVectorValue());
        
    }
    
    if(inlet == 0){
        // check if all input have been feeded at least once
        // we never call inputStruct.resetcounter on cold inlet, so updated represent at least one value has been recieved
        hasInput = true;
        for(auto i:inputStruct){
            hasInput&=i.isUpdated();
        }
        
        compute();
        if(outHopSize<=0 || inputStruct[0].hasReachedHop(outHopSize))
            outputIt(NULL);
        
    }
    
}

#endif



void essentiaRT2::my_bang() {
    outputIt(NULL);
}

void essentiaRT2::outputIt(void *){
    if(hasInput){
        for(int i = outputStruct.size()-1;i>=0 ; i--){
            if(!outputStruct[i].isUpdated()){
                log("not Updated : " + outputStruct[i].name);
            }
            else{
                if(outputStruct[i].type==Helper::ioStruct::VECTOR ){
                        ToOutList(i,outputStruct[i].aggregateVector());
                    
                }
                else if (outputStruct[i].type == ioStruct::STRING){
                    ToOutAtom(i, outputStruct[i].aggregateString());
                }
                else{
                    ToOutAtom(i, outputStruct[i].aggregateReal());
                    
                }
                outputStruct[i].resetCounter(outHopSize);
                
            }
            
        }
#ifndef FLEXT_TILDE
        inputStruct[0].resetCounter(outHopSize);
#endif
    }
    
};




void essentiaRT2::buildAlgo(){
    
    myAlgo = essentia::standard::AlgorithmFactory::create(name);
#ifdef FLEXT_TILDE
    isSpectrum = false;
    for(auto in:myAlgo->inputs()){
        if (in.first == "spectrum" ){
            isSpectrum = true;
            frameSize/=2;
            frameSize++;
            hopSize/=2;
            hopSize++;
        }
    }
#endif
    vector<string> paramNames = myAlgo->defaultParameters().keys();
    
    if (std::find(paramNames.begin(), paramNames.end(), "frameSizeameRate") != paramNames.end()){
        myAlgo->configure("frameSizeameRate",sampleRate*1.0/hopSize);
    }
    if (std::find(paramNames.begin(), paramNames.end(), "sampleRate") != paramNames.end()){
        myAlgo->configure("sampleRate",sampleRate);
    }
    if (std::find(paramNames.begin(), paramNames.end(), "hopSizeSize") != paramNames.end()){
        myAlgo->configure("hopSizeSize",hopSize);
    }
    if (std::find(paramNames.begin(), paramNames.end(), "frameSizeameSize") != paramNames.end()){
        myAlgo->configure("frameSizeameSize",frameSize);
    }
    
    for(auto a:paramsS){
        myAlgo->configure(a.first,a.second);
    }
    for(auto a:paramsF){
        myAlgo->configure(a.first,a.second);
    }
    int inIdx = 0;
#ifdef FLEXT_TILDE
    inputVectors.resize(myAlgo->inputs().size());
#else
    inputStruct.resize(myAlgo->inputs().size());
#endif
    
#ifdef FLEXT_TILDE
    outSize = outSize>0? outSize  *  sampleRate*1.0/hopSize + 10 : 1;
#else
    if(outSize<=0)outSize = 1;
#endif
    
    for(auto in:myAlgo->inputs()){
        if(in.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            
#ifdef FLEXT_TILDE
            inputVectors[inIdx]  = vector<Real>(frameSize,0);
            AddInSignal(in.first.c_str());
            in.second->set(inputVectors[inIdx]);
#else
            inputStruct[inIdx] = ioStruct(in.first,ioStruct::VECTOR,outSize);
            AddInList(in.first.c_str());
#endif
        }
        else if(in.second->typeInfo() ==typeid(essentia::Real)){
            
#ifdef FLEXT_TILDE
            err("Real input notAllowed in tilde mode ignoring input : " + in.first + " , use essentiaRT2 object");
#else
            inputStruct[inIdx] = ioStruct(in.first,ioStruct::REAL,outSize);
            AddInFloat(in.first.c_str());
#endif
        }
        else if( in.second->typeInfo() ==typeid(std::vector< std::vector< essentia::Real> > )){
            err("matrix not supported");
            
#ifndef FLEXT_TILDE
            inputStruct[inIdx] = ioStruct(in.first,ioStruct::MATRIX,outSize);
            AddInList(in.first.c_str());
#endif
        }
        
        
        inIdx++;
    }
    
    outputStruct.resize(myAlgo->outputs().size());
    int outI = 0;

    
    for(auto out:myAlgo->outputs()){
        if(out.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            outputStruct[outI] = ioStruct(out.first,ioStruct::VECTOR,outSize);
            AddOutList(out.first.c_str());
            out.second->set(outputStruct[outI].getNextVectorValue());
        }
        else if(out.second->typeInfo() ==typeid( essentia::Real)){
            outputStruct[outI] = ioStruct(out.first,ioStruct::REAL,outSize);
            AddOutFloat(out.first.c_str());
            out.second->set(outputStruct[outI].getNextRealValue());
        }
        else if( out.second->typeInfo() ==typeid(string )){
            
            outputStruct[outI] = ioStruct(out.first,ioStruct::STRING,outSize);
            AddOutSymbol(out.first.c_str());
            out.second->set(outputStruct[outI].getNextStringValue());
            
        }
        else {
            outputStruct[outI] = ioStruct(out.first,ioStruct::MATRIX,outSize);
            AddOutFloat(out.first.c_str());
            log("cant parse outlet of type : " + essentia::nameOfType(out.second->typeInfo()));
        }

        outI++;
        
    }
    
    getAlgoDescription();
}


bool essentiaRT2::compute(){
    if(hasInput){
        for(int i = 0 ; i < outputStruct.size() ; i++){
            
            if(outputStruct[i].type==ioStruct::VECTOR)
            {myAlgo->output(outputStruct[i].name).set(outputStruct[i].getNextVectorValue());}
            
            else if (outputStruct[i].type==ioStruct::REAL)
            {myAlgo->output(outputStruct[i].name).set(outputStruct[i].getNextRealValue());}
            else if (outputStruct[i].type==ioStruct::STRING)
            {
                myAlgo->output(outputStruct[i].name).set(outputStruct[i].getNextStringValue());
            }
            // dumb operation for allowing essentia using algos with matrices (not outputed in pd)
            else if (outputStruct[i].type==ioStruct::MATRIX)
            {myAlgo->output(outputStruct[i].name).set(outputStruct[i].vectorValues);
                log("matrix not supported undefined behavior on outlet : "+std::to_string(i));
            }
        }
        myAlgo->compute();
    }
}


void essentiaRT2::parseArgs(int argc, const t_atom *argv){
    if(argc == 0){err ( "no argument provided" );return;}
    int argIdx = 0;
    
    while (argIdx < argc &&IsFloat(argv[argIdx]) && !IsString(argv[argIdx])) {
        
        switch (argIdx) {
            case 0:
                frameSize=GetFloat(argv[argIdx]);
                break;
            case 1:
                hopSize=GetFloat(argv[argIdx]);
                
            default:
                break;
        }
        argIdx++;
        
    }
    if(!IsString(argv[argIdx])){
        err("no valid name found");
    }
    else{
        name = GetString(argv[argIdx]);
        argIdx++;
    }
    
    if(IsFloat(argv[argIdx])){
        outSize=GetFloat(argv[argIdx]);
        argIdx++;
    }
    outHopSize = outSize;
    if(IsFloat(argv[argIdx])){
        outHopSize=GetFloat(argv[argIdx]);
        argIdx++;
    }
    
    int beginMsg = argIdx;
    string curName = "";
    while(argIdx < argc){
        if((argIdx - beginMsg)%2==0){
            if(!IsString(argv[argIdx])){err("no valid parameter : [name value] list");}
            else{ curName = GetAString(argv[argIdx]);}
        }
        else{
            // specific arguments
            if(curName[0] == '_'){
                if(curName == "_aggr"){
                    string arg = GetAString(argv[argIdx]);
                    std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
                    ioStruct::AggrType type = arg == "median"?ioStruct::AggrType::MEDIAN:ioStruct::AggrType::MEAN;
                    for (auto  t:outputStruct){
                        t.aggrType = type;
                    }
                }

            }
            else{
                if(IsString(argv[argIdx])){paramsS[curName] = GetString(argv[argIdx]);}
                else if(IsFloat(argv[argIdx])){paramsF[curName] = GetFloat(argv[argIdx]);}
            }
        }
        argIdx++;
        
    }
    
    
    
    
}



void essentiaRT2::getAlgoDescription(){
    ostringstream description;
    description << name + "\n";
    description<< " input :\n";
    int idx = 0;
    for( auto kv:myAlgo->inputDescription){
        description<<"  "+kv.first + "("+nameOfType(myAlgo->input(kv.first))+") :\n   " + kv.second+"\n";
        ;
    }
        description<< " output :\n";
    
    for( auto kv:myAlgo->outputDescription){
        description<<"  "+kv.first + "("+nameOfType(myAlgo->output(kv.first))+") :\n   " + kv.second+"\n";
    }
    description << " parameters :\n";
    for( auto kv:myAlgo->parameterDescription){
        description<<"  "+kv.first +"("<<myAlgo->parameter(kv.first).type()<<") :\n   " + kv.second+"\n";
        
    }
    
    post(description.str().c_str());

}

void essentiaRT2::getAllAlgos(){
    string names;
    //ordered and uppercase ordered keys;
    char lastLetter = 'A';
    for(auto n:essentia::standard::AlgorithmFactory::keys()){
        if(n[0]!=lastLetter){
            post(names.c_str());
            names = "";
            lastLetter = n[0];
        }
        names+=n+", ";
    }
    post(names.c_str());
}




