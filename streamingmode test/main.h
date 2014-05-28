//
//  main.h
//  superFlux~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//

#ifndef superFlux__main_h
#define superFlux__main_h


/*
 flext tutorial - simple 2
 
 Copyright (c) 2002,2003 Thomas Grill (xovo@gmx.net)
 For information on usage and redistribution, and for a DISCLAIMER OF ALL
 WARRANTIES, see the file, "license.txt," in this distribution.
 
 -------------------------------------------------------------------------
 
 This is an example of a simple object doing a float addition
 */

#define FLEXT_USE_CMEM
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

#include "Essentia.h"


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


class superFlux : public flext_dsp
{
public:
    FLEXT_HEADER(superFlux, flext_dsp)

    superFlux();
    ~superFlux();

    void m_signal(int n, t_sample *const *insigs, t_sample *const *outsigs);    
    void my_bang();

    void outputListOfFeatures(const std::map<string, vector<Real> >& features);
    
    int essentiaBufferCounter;
    vector<Real> audioBuffer,audioOutput;
    
    Essentia essentia;


    FLEXT_CALLBACK(my_bang);
};
FLEXT_NEW_DSP("superFlux~", superFlux)


#endif
