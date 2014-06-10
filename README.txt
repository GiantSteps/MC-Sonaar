
essentiaRT~
===========


Created By:
----------- 
Martin Hermant, Carthach O’Nuanain and Ángel Faraldo.
Music Technology Group
Universitat Pompeu Fabra
Barcelona, Spain
http://,tg.upf.edu


Description
-----------

essentiaRT~ is a real-time subset of Essentia (MTG's open-source C++ library for audio analysis and audio-based music information retrieval) implemented as an external for Pd and Max/MSP. As such, the current version does not yet include all of Essentia’s algorithms, but a number of features to slice and provide on-the-fly descriptors for classification of audio in real-time. In the context of the GiantSteps EU-funded project, we expect to add further functionalities, including (but not being limited to) more algorithms currently in Essentia.

At the core of essentiaRT~ lays Sebastian Böck's onset-detection algorithm SuperFlux, recently added to Essentia's arsenal of algorithms. On top of this, a number of Essentia extractors analyse instantaneous features like the onset strength, the spectral centroid and the MFCC's over a fixed-size window of 2048 points, after an onset is reported. Furthermore, essentiaRT~ is able to perform estimations on larger time-frames of user-defined lengths, and to report finer descriptions in terms of noisiness, f0, temporal centroid and loudness.

essentiaRT~ is provided as a binary file compiled for Linux, Mac OSX, and Windows. Additionally, it comes with a collection of abstractions designed to make interaction with the object easier. A number of help files and use examples complete the package.

EssentiaRT~ is offered free of charge for non-commercial use only.

Plug-in authored by Martin Hermant, Cárthach Ó'Nuanáin and Ángel Faraldo.


References
----------

The user is referred to the following sources for detailed descriptions of SuperFlux and Essentia:

Böck S. & Widmer G. (2013). 'Maximum Filter Vibrato Suppression for Onset Detection.' In Proceedings of the 16th International Conference on Digital Audio Effects, Maynooth, Ireland, September 2013.

Bogdanov, D., Wack N., Gómez E., Gulati S., Herrera P., Mayor O., et al. (2013). 'ESSENTIA: an Audio Analysis Library for Music Information Retrieval.' International Society for Music Information Retrieval Conference (ISMIR'13). 493-498.


Object Description
------------------

- Creation argument: This is the threshold level for the onset detector. It will typically be in the range 10 and 50, depending on the source. With a higher threshold, the object will only report very prominent attacks, such as those produced by percussive instruments.

- inlet 1: audio signal

- outlet 1: Onset detector novelty function at audio rate.

- outlet 2: list of instantaneous descriptors calculated over the 2048 samples following the detection of an onset. They include:
​
--- i.centroid: spectral centroid.

--- i.mfcc: list of 13 Mel Frecuency Cepstral Coefficients.

--- i.strength: strength of the onset
​
outlet 3: list of averaged features (mean and variance). These values are estimated over a window size specified by the user with the method "delayMode". The estimated features are:
​centroid: spectral centroid.

--- f0: estimation of the fundamental frequency

--- f0Confidence: confidence measure on the f0 estimation.

--- loudness: loudness of the excerpt.

--- mfcc: Mel Frecuency Cepstral Coefficients averaged over the full slice.

--- noisiness: a measure of the flatness in the spectrum. It can tell us whether a sound is pitched or un-pitched (i.e. noisy).

--- tempCentroid: Temporal centroid. It calculates where the energy concentrates in the analised chunk. 
​
Methods: DelayMode. Once an onset is reported (on the 2nd outlet), a larger window is used to estimate some of other descriptors of the event, and then listed in outlet3. The window size over which these parameters are calculated can be set dynamically with the method "delayMode"  followed by a scalar value in ms. A delayMode time of 0 estimates over the full audio chunk between onset reports.


System Requirements
-------------------

A computer with Pd-extended (version 0.42.5 or newer).

Currently, this external is will NOT work on intel core2duo processors.


Installation
------------

Linux:
=====

1) Install Dependencies. You need to install the dependencies of Essentia (you don't actually need to build/install Essentia itself). Do this on Debian with: ​

sudo apt-get install build-essential libyaml-dev libfftw3-dev libavcodec-dev libavformat-dev python-dev libsamplerate0-dev libtag1-dev

2) Use the appropriate external (Only for 32-bit Linux).

​​If you're on 64-bit you don't have to do anything.
If you're on 32-bit, rename "essentiaRT~.pd_linux32" to "essentiaRT~.pd_linux"

OSX:
===

Add the essentiaRT~ folder to the Pd path (in Pd > Preferences) or copy the folder with the binaries, the examples and the utils where Pd can find it (typically to ~/Library/Pd or to the /extra folder inside Pd (Pd-Extended > Show Package Contents > Contents/Resources/extra).

Windows:
=======

Copy the all the dlls in the "/dependencies/win32" folder to one of the following:

C:\Windows\System32
The same folder that the essentiaRT~ resides in.


Acknowlegdements
-----------------

EssentiaRT~ is build upon the work of Sebastian Böck (SuperFlux), Dmitry Bogdanov (Essentia) and the Pd and Pd-extended community. 

EssentiaRT~ was developed by Martin Hermant, Cárthach Ó'Nuanáin and Ángel Faraldo within the Music Technology Group.

The related research was carried out in the frame of the GiantSteps project which is partly funded by the  European Commission (Seventh Framework Programme for research, technological development and demonstration under grant agreement no 610591) and the TECNIO network promoted by ACC1Ó agency by the Catalan Government.


© Copyright 2014 Music Technology Group, Universitat Pompeu Fabra. All Rights Reserved.