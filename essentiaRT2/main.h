//
//  main.h
//  essentiaRT2~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//

#ifndef essentiaRT2__main_h
#define essentiaRT2__main_h


/*
 flext tutorial - simple 2
 
 Copyright (c) 2002,2003 Thomas Grill (xovo@gmx.net)
 For information on usage and redistribution, and for a DISCLAIMER OF ALL
 WARRANTIES, see the file, "license.txt," in this distribution.
 
 -------------------------------------------------------------------------
 
 This is an example of a simple object doing a float addition
 */

//#define FLEXT_ATTRIBUTES 1

// include flext header
#include <flext.h>
#include "math.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
using std::vector;

#include "essentia/essentia.h"
#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/pool.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/debugging.h>

using namespace std;
using namespace essentia;
using namespace essentia::standard;




// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif


namespace Helper {

    flext::AtomList floatVectorToList(const std::vector<float>& floatVector) {
        flext::AtomList listOut(floatVector.size());
        
        for(int i=0; i<floatVector.size(); i++) {
            t_atom value;
            flext::SetFloat(value, floatVector[i]);
            
            listOut[i] = value;
        }
        return listOut;
    }

    flext::AtomList stringVectorToList(const std::vector<std::string>& stringVector) {
        flext::AtomList listOut(stringVector.size());
        
        for(int i=0; i<stringVector.size(); i++) {
            t_atom value;
            flext::SetString(value, stringVector[i].c_str());
            
            listOut[i] = value;
        }
        return listOut;
    }
    
    std::vector<float> listToVector(int argc , const t_atom * argv) {
        std::vector<float> listOut(argc);
        for(int i=0; i<argc; i++) {
            listOut[i] = flext::GetAFloat(argv[i]);
        }
        return listOut;
    }
    


};


class essentiaRT2 : public flext_dsp
{
public:
    FLEXT_HEADER_S(essentiaRT2, flext_dsp, setup)

    essentiaRT2(int argc,const t_atom *argv);
    ~essentiaRT2();
//// Flext

    void my_bang();
    void parseArgs(int argc,const t_atom *argv);
    void buildAlgo();
    
    ///// Essentia Config
    //void outputListOfFeatures(const std::map<string, vector<Real> >& features);
    int sampleRate ;
    int frameSize ;
    int hopSize;
    
    typedef struct{

        int fR=1024;
        int hop=1024;
        string name="HPCP";
        double outRate=0;
        map<string,string> paramsS;
        map<string,float> paramsF;

        
    }algo;
    algo curAlgo;
    

    
protected:

    //Essentia algo
    essentia::standard::Algorithm* myAlgo;
    essentia::standard::Algorithm* FC;
    
    
    typedef struct ioStruct{
        string name;
        bool isVector;
        vector<Real> realValues;
        vector< vector<Real> > vectorValues;
        int idxCount = 0;
        int maxSize = 100;
        int beginIdx = 0;
        int curVecSize = 0;
        ioStruct(){
        }
        ioStruct(string _name,bool _isVector,int num = 100):name(_name),isVector(_isVector){
            init(num);
        }
        void init(int num){
            if(isVector){
                vectorValues.resize(num);
            }
            else{
            realValues.resize(num);
            }
            idxCount=0;
            maxSize = num;
            beginIdx = 0;
        }
        Real & getNextRealValue(){
            idxCount%=maxSize;
            idxCount++;
            return realValues[idxCount-1];
        }
        Real & getRealValue(){
            return realValues[idxCount-1];
        }
        vector<Real> & getVectorValue(){
            return vectorValues[idxCount-1];
        }
        
        vector<Real> & getNextVectorValue(int s){
            idxCount%=maxSize;
            idxCount++;
            vectorValues[idxCount-1].resize(s);
            if(s!=curVecSize){
                for(int i = 0;i<idxCount-beginIdx ; i++){
                    vectorValues[(i+beginIdx)%vectorValues.size()].resize(s);
                }
                curVecSize = s;
                post ("resizing vector");
            }
            return vectorValues[idxCount-1];
        }
        
        vector<Real> & getNextVectorValue(){
            idxCount%=maxSize;
            idxCount++;
            return vectorValues[idxCount-1];
        }
        
        Real aggregateReal(){
            
            return essentia::mean<Real>(realValues,beginIdx,idxCount);
        }
        vector<Real> aggregateVector(){
            vector<Real> res(vectorValues[beginIdx].size());
            for(int i = 0;i<idxCount-beginIdx ; i++){
                for(int j = 0 ; j < res.size() ; j ++ ){
                    res[j]+=vectorValues[(i+beginIdx)%vectorValues.size()][j];
                }
                
            }
            for(int j = 0 ; j < res.size() ; j ++ ){
                res[j]/=vectorValues[beginIdx].size();
            }
            return res;
           
        }
        void resetCounter(){post("resetting : ");
                                 post(std::to_string(beginIdx-idxCount).c_str());beginIdx = idxCount;};
        
        
    }ioStruct;
    
    
    vector<ioStruct> inputStruct,outputStruct;

    
    ///essentia Environement
    int essentiaBufferCounter;
    vector<Real> audioBuffer,audioBufferOut;
    std::map<string, vector<Real> > getFeatures(Pool p);
    void outputListOfFeatures(const std::map<string, vector<Real> >& features,int outlet = 1);
    
    
    Pool pool;

    virtual bool CbMethodResort(int inlet,const t_symbol *s,int argc,const t_atom *argv);
    
    flext::Timer OutTimer;
    void outputIt(void *);

private:
    static void setup(t_classid c);

    FLEXT_CALLBACK(my_bang)
    FLEXT_CALLBACK_T(outputIt)

    int blockCount = 0;
    int blockCountMax = 1;
    
    static bool inited ;

    

    

    

};
FLEXT_NEW_V("essentiaRT2", essentiaRT2)
#endif
