
===================================== essentiaRT~ ==================================

Description
———————————

essentiaRT~ is a real-time implementation of MTG’s Essentia (an open-source C++ library for audio analysis and audio-based music information retrieval) as Pd and Max/MSP objects. As such, it does not yet include all of Essentia’s algorithms, but a number of featyure extractors considered useful in a real-time scenario to provide on-the-fly classification of sounds.

<argument>: threshold level for the Super-Flux algorithm.

<in_1>: audio signal

<out_1>: onset strength value at audio rate (constant until a new onset is detected).

<out_2>: list of instantaneous descriptors calculated over the 2048 samples immediately after an onset is detected

    - strength: onset strength (same as in outlet1).
    
    - centroid: spectral centroid.
    
    - mfcc: 
    

<out_3>: list of “higher-level features”. These values are estimated over a bigger window (100 ms??) once an onset is detected:

    - noisiness: this gives the mean of the ‘noisiness’ of the sound by measuring the peakiness in the spectrum.

    - f0

    - chroma

    - loudness

    - mfcc

    - centroid

    - log-attack time

    - all the high level features have their dependent variance: noisiness.var, f0.var, etc.
    
    - nsdf.mean: mean of the normalised squared difference function over the analysis window.

    - nsdf.var: and its variance.

    - pitched.mean: this gives the mean of the 'pitchniness' of the sound by measuring the peakiness in the spectrum.

    - pitched.var: and its variance.