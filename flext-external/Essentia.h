//
//  Essentia.h
//  pd_essentia~
//
//  Created by Cárthach Ó Nuanáin on 15/05/2014.
//
//

#ifndef __pd_essentia___Essentia__
#define __pd_essentia___Essentia__

#include <iostream>
#include <map>

#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/pool.h>

#include "flext.h"


using namespace std;
using namespace essentia;
using namespace essentia::standard;

class Essentia {

public:
    std::map<string, bool> currentAlgorithms;

    Essentia();
    ~Essentia();
    
    void setup(int sampleRate, int frameSize, int hopSize);
    void compute(vector<Real> audioFrame);
//    vector<flext::AtomList> getFeatures();
//    void getFeatures();
    std::map<string, vector<Real> > getFeatures();
    
    //////ALGS////////
    Algorithm *fc, *w, *spec, *mfcc, *loud;
    
    /////// PARAMS //////////////
    int sampleRate, frameSize, hopSize;
    
    /////STRUCTURES//////    
    Pool pool;
};

#endif /* defined(__pd_essentia___Essentia__) */
