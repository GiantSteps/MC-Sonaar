//
//  main.h
//  essentiaRT~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//

#ifndef essentiaRT__main_h
#define essentiaRT__main_h


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
#include <essentia/debugging.h>

using namespace std;
using namespace essentia;
using namespace essentia::standard;


#include "EssentiaOnset.h"
#include "EssentiaSFX.h"

// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif

//stop aggregating after 6s if sfx are set on ioi (delMode = 0)
#define MAX_SFX_TIME 6
#define THREADED_SFX

#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif	/* MAX */

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


class essentiaRT : public flext_dsp
{
public:
    FLEXT_HEADER_S(essentiaRT, flext_dsp, setup)

    essentiaRT(int argc,const t_atom *argv);
    ~essentiaRT();
//// Flext
    void CbSignal();//void m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs);
    void CbSignal64();
    void my_bang();

    
    ///// Essentia Config
    //void outputListOfFeatures(const std::map<string, vector<Real> >& features);
    int sampleRate ;
    int frameSize ;
    int hopSize;
    
    
    

    
protected:

    
    //Essentia core
    EssentiaOnset   onsetDetection;
    Real onset_thresh;
    
    EssentiaSFX   SFX;
    long sfxCount;

    int delayMode = 0;
    
    ///essentia Environement
    int essentiaBufferCounter;
    vector<Real> audioBuffer,audioBufferOut;
    std::map<string, vector<Real> > getFeatures(const Pool & p);
    
    void outputListOfFeatures(const std::map<string, vector<Real> >& features,int outlet = 1);
    

    void compute();
    
    

    void m_sfxAggr(void *);
    void m_delayMode(int del);
    void m_threshold(float thresh);
    void m_rthreshold(float thresh);
    void m_combine(float c);
    int checkBlockSize();

    flext::Timer SFXTimer;
    bool isComputingSFX,isAggregatingSFX;

private:
    static void setup(t_classid c);
    void  onsetCB();
    FLEXT_CALLBACK(onsetCB)

    FLEXT_CALLBACK(my_bang)
    FLEXT_CALLBACK_T(m_sfxAggr);
    FLEXT_CALLBACK_I(m_delayMode);
    FLEXT_CALLBACK_F(m_threshold);
    FLEXT_CALLBACK_F(m_rthreshold);
    FLEXT_CALLBACK_F(m_combine);

    
    void aggrThreadFunc();
    
    std::thread aggrThread;
    

};
FLEXT_NEW_DSP_V("essentiaRT~", essentiaRT)
#endif
