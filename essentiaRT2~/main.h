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
using std::vector;

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

//stop aggregating after 10s if sfx are set on ioi (delMode = 0)
#define MAX_SFX_TIME 10

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
    
    Real getReal(t_atom a){

       return 0.0;
    }

};


class essentiaRT2 : public flext_dsp
{
public:
    FLEXT_HEADER_S(essentiaRT2, flext_dsp, setup)

    essentiaRT2(int argc,const t_atom *argv);
    ~essentiaRT2();
//// Flext
    void m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs);    
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
    essentia::streaming::Algorithm* myAlgo;
    essentia::streaming::Algorithm* FC;

    
    ///essentia Environement
    int essentiaBufferCounter;
    vector<Real> audioBuffer,audioBufferOut;
    std::map<string, vector<Real> > getFeatures(Pool p);
    void outputListOfFeatures(const std::map<string, vector<Real> >& features,int outlet = 1);
    
    
    Pool pool;

    
    
    flext::Timer OutTimer;
    void outputIt(void *);

private:
    static void setup(t_classid c);

    FLEXT_CALLBACK(my_bang)
    FLEXT_CALLBACK_T(outputIt)

    int blockCount = 0;
    int blockCountMax = 1;
    
    

    

};
FLEXT_NEW_DSP_V("essentiaRT2~", essentiaRT2)
#endif
