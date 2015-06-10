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

    
}




void essentiaRT2::my_bang() {
    outputIt(NULL);
}

void essentiaRT2::outputIt(void *){
    int i = 0;
    post("outputing");
    for(auto out:outputStruct){
        if(out.isVector){
            cout << outputStruct[i].aggregateVector() << endl;;
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
   
    myAlgo = essentia::standard::AlgorithmFactory::create(curAlgo.name);
    post("building Algo");
    for(auto a:curAlgo.paramsS){
        myAlgo->configure(a.first,a.second);
    }
    for(auto a:curAlgo.paramsF){
        myAlgo->configure(a.first,a.second);
    }
    post(" ");
    post("inputs:");
    int inIdx = 0;
    for(auto in:myAlgo->inputs()){
        
        post(in.first.c_str());
        post(nameOfType(in.second->typeInfo()).c_str());
        AddInAnything(in.first.c_str());
        if(in.second->typeInfo() ==typeid( std::vector<essentia::Real>)){
            inputStruct.push_back(ioStruct(in.first,true));
//            AddInList(in.first.c_str());
        }
        else if(in.second->typeInfo() ==typeid(essentia::Real)){
            inputStruct.push_back(ioStruct(in.first,false));
//            AddInFloat(in.first.c_str());
        }
//        inIdx++;
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
    
    
    if(curAlgo.outRate!=0){
        OutTimer.Periodic(curAlgo.outRate);
    }
}

bool essentiaRT2::CbMethodResort(int inlet, const t_symbol *s, int argc, const t_atom *argv){

    if(!inputStruct[inlet].isVector){
        if(argc>1){
            post("too much element on inlet");
            post(std::to_string(inlet).c_str());
        }
        else{
        inputStruct[inlet].getNextRealValue() = GetAFloat(argv[0]);
        myAlgo->input(inputStruct[inlet].name).set(inputStruct[inlet].getRealValue());
        }
    }
    else {
        
        inputStruct[inlet].getNextVectorValue(argc);
        inputStruct[inlet].getVectorValue() = Helper::listToVector(argc,argv);
        
        myAlgo->input(inputStruct[inlet].name).set(inputStruct[inlet].getVectorValue());
    }
    
    if(inlet == 0){
        for(int i = 0 ; i < outputStruct.size() ; i++){
            if(outputStruct[i].isVector){
                myAlgo->output(outputStruct[i].name).set(outputStruct[i].getNextVectorValue());
            }
            else{
                myAlgo->output(outputStruct[i].name).set(outputStruct[i].getNextRealValue());
            }
        }
        
        myAlgo->compute();
        outputIt(NULL);
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




