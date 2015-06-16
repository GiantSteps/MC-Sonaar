#include "main.h"
#include "essentia/essentia.h"


bool essentiaRT2::debug = false;

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
    FLEXT_ADDTIMER(OutTimer,outputIt);
    
    log("creating");
    /////// PARAMS //////////////
    sampleRate = Samplerate();
    frameSize=Blocksize();
    hopSize=Blocksize();
    
    
    outRate = -1;
    aggrSize = 0;
    
    
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
                    compute();
                    if(outRate==-1){
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
        compute();
        if(outRate<=0)
            outputIt(NULL);
    }
    
}

#endif



void essentiaRT2::my_bang() {
    outputIt(NULL);
}

void essentiaRT2::outputIt(void *){
    
    for(int i = outputStruct.size()-1;i>=0 ; i--){
        if(!outputStruct[i].isUpdated()){
            log("not Updated : " + outputStruct[i].name);
        }
            if(outputStruct[i].type==Helper::ioStruct::VECTOR ){
                ToOutList(i,Helper::floatVectorToList(outputStruct[i].aggregateVector()));
            }
            else if (outputStruct[i].type == ioStruct::STRING){
                ToOutString(i, outputStruct[i].aggregateString().c_str());
            }
            else{
                ToOutFloat(i, outputStruct[i].aggregateReal());
                
            }
            outputStruct[i].resetCounter();

        
    
    }
    
};




void essentiaRT2::buildAlgo(){
    
    myAlgo = essentia::standard::AlgorithmFactory::create(name);
    log("building Algo : " + name);
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

    log("inputs:");
    int inIdx = 0;
#ifdef FLEXT_TILDE
    inputVectors.resize(myAlgo->inputs().size());
#else
    inputStruct.resize(myAlgo->inputs().size());
#endif
    

    
    for(auto in:myAlgo->inputs()){
        log("   " +in.first +" : "+ nameOfType(in.second->typeInfo()));
        if(in.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            
#ifdef FLEXT_TILDE
            inputVectors[inIdx]  = vector<Real>(frameSize,0);
            AddInSignal(in.first.c_str());
            in.second->set(inputVectors[inIdx]);
#else
            inputStruct[inIdx] = ioStruct(in.first,ioStruct::VECTOR);
            AddInList(in.first.c_str());
#endif
        }
        else if(in.second->typeInfo() ==typeid(essentia::Real)){
            
#ifdef FLEXT_TILDE
            err("Real input notAllowed in tilde mode ignoring input : " + in.first + " , use essentiaRT2 object");
#else
            inputStruct[inIdx] = ioStruct(in.first,ioStruct::REAL);
            AddInFloat(in.first.c_str());
#endif
        }
        else if( in.second->typeInfo() ==typeid(std::vector< std::vector< essentia::Real> > )){
            err("matrix not supported");
            
#ifndef FLEXT_TILDE
            inputStruct[inIdx] = ioStruct(in.first,ioStruct::MATRIX);
            AddInList(in.first.c_str());
#endif
        }
        

        inIdx++;
    }

    log("outputs:");
    outputStruct.resize(myAlgo->outputs().size());
    int outI = 0;
#ifdef FLEXT_TILDE
    aggrSize = outRate>0? outRate  *  sampleRate*1.0/hopSize + 10 : 1;
#else
    if(aggrSize==0)aggrSize = 1;
#endif
    
    for(auto out:myAlgo->outputs()){
        log("   "+out.first +"  : " + nameOfType(out.second->typeInfo()));
        if(out.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            outputStruct[outI] = ioStruct(out.first,ioStruct::VECTOR,aggrSize);
            AddOutList(out.first.c_str());
            out.second->set(outputStruct[outI].getNextVectorValue());
        }
        else if(out.second->typeInfo() ==typeid( essentia::Real)){
            outputStruct[outI] = ioStruct(out.first,ioStruct::REAL,aggrSize);
            AddOutFloat(out.first.c_str());
            out.second->set(outputStruct[outI].getNextRealValue());
        }
        else if( out.second->typeInfo() ==typeid(string )){

            outputStruct[outI] = ioStruct(out.first,ioStruct::STRING,aggrSize);
            AddOutSymbol(out.first.c_str());
            out.second->set(outputStruct[outI].getNextStringValue());
        
        }
        else {
            outputStruct[outI] = ioStruct(out.first,ioStruct::MATRIX,aggrSize);
            AddOutFloat(out.first.c_str());
            log("cant parse outlet of type : " + essentia::nameOfType(out.second->typeInfo()));
        }
        
            
            

        
        outI++;
        
    }
    
    
    if(outRate>0){
        OutTimer.Periodic(outRate);
    }
    
}


bool essentiaRT2::compute(){
    
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


void essentiaRT2::parseArgs(int argc, const t_atom *argv){
    if(argc == 0){err ( "no argument provided" );return;}
    int argIdx = 0;
    
    while (argIdx < argc &&IsFloat(argv[argIdx]) && !IsString(argv[argIdx])) {
        
        switch (argIdx) {
            case 0:
                frameSize = GetFloat(argv[argIdx]);
                break;
            case 1:
                hopSize = GetFloat(argv[argIdx]);
                
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
        outRate =GetFloat(argv[argIdx])/1000.0;
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
                if(curName == "_aggrS"){
                    aggrSize = GetAInt(argv[argIdx]);
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




