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


#include "helper.h"

using namespace std;
using namespace essentia;
using namespace essentia::standard;
using namespace Helper;




// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif





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
    
    int fR;
    int hop;
    string name="HPCP";
    double outRate;
    map<string,string> paramsS;
    map<string,float> paramsF;
 
    bool compute();
    
    
    
protected:
    
    //Essentia algo
    essentia::standard::Algorithm* myAlgo;
    
    vector<ioStruct> outputStruct;
    vector < vector < Real> > inputVectors;
    
    ///essentia Environement
    
    int audioBufferCounter;
    vector<Real> audioBuffer;
    
    
    flext::Timer OutTimer;
    void outputIt(void *);
    
private:
    static void setup(t_classid c);
    
    FLEXT_CALLBACK(my_bang)
    FLEXT_CALLBACK_T(outputIt)
    
    static bool inited ;
    
    
    
    
    
    
    
};
FLEXT_NEW_DSP_V("essentiaRT2~", essentiaRT2)
#endif