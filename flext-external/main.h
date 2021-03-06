//
//  main.h
//  pd_essentia~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//

#ifndef pd_essentia__main_h
#define pd_essentia__main_h


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

using namespace std;
using namespace essentia;
using namespace essentia::standard;

#include "EssentiaPD.h"


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
};


class pd_essentia : public flext_dsp
{
public:
    FLEXT_HEADER_S(pd_essentia, flext_dsp, setup)

    pd_essentia(int argc,const t_atom *argv);
    ~pd_essentia();

    void m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs);    
    void my_bang();

    void outputListOfFeatures(const std::map<string, vector<Real> >& features);
    
    int essentiaBufferCounter;
    vector<Real> audioBuffer;
    
    EssentiaPD essentia;
    
protected:
    virtual void m_features(int argc, const t_atom *argv);
    
    // timers
	Timer tmrA;
    
	void m_timerA(void *);
    
	void m_resetA();

	void m_oneshotA(int del);

	void m_periodicA(int del);
    
    bool listNotAlreadyOut = true;


private:
    static void setup(t_classid c);

    FLEXT_CALLBACK_V(m_features)
    FLEXT_CALLBACK(my_bang)
    
    // register timer callbacks
	FLEXT_CALLBACK_T(m_timerA)
	
	// register method callbacks
	FLEXT_CALLBACK(m_resetA)
	FLEXT_CALLBACK_I(m_oneshotA)
	FLEXT_CALLBACK_I(m_periodicA)
};
FLEXT_NEW_DSP_V("pd_essentia~", pd_essentia)
#endif
