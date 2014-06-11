
essentiaRT~
===========

Created By:
----------- 
Martin Hermant, Cárthach Ó Nuanáin and Ángel Faraldo.
Music Technology Group
Universitat Pompeu Fabra
Barcelona, Spain
http://mtg.upf.edu


Description
-----------

essentiaRT~ is a real-time subset of Essentia (MTG's open-source C++ library for audio analysis and audio-based music information retrieval) implemented as an external for Pd and Max/MSP. As such, the current version does not yet include all of Essentia’s algorithms, but a number of features to slice and provide on-the-fly descriptors for classification of audio in real-time. In the context of the GiantSteps EU-funded project, we expect to add further functionalities, including (but not being limited to) more algorithms currently in Essentia.

At the core of essentiaRT~ lies Sebastian Böck's onset-detection algorithm SuperFlux, recently added to Essentia's library of algorithms. In addition, a number of Essentia extractors analyse instantaneous features such as onset strength, spectral centroid and MFCCs over a fixed-size window of 2048 points, after an onset is reported. Furthermore, essentiaRT~ is able to perform estimations on larger time-frames of user-defined lengths, and to report finer descriptions in terms of noisiness, f0, temporal centroid and loudness.

essentiaRT~ is provided as a binary file compiled for Linux, Mac OSX, and Windows. Additionally, it comes with a collection of abstractions designed to make interaction with the object easier. A number of help files and examples for use complete the package.

EssentiaRT~ is offered free of charge for non-commercial use only.


References
----------

The user is referred to the following sources for detailed descriptions of SuperFlux and Essentia:

Böck S. & Widmer G. (2013). 'Maximum Filter Vibrato Suppression for Onset Detection.' In Proceedings of the 16th International Conference on Digital Audio Effects, Maynooth, Ireland, September 2013.

Bogdanov, D., Wack N., Gómez E., Gulati S., Herrera P., Mayor O., et al. (2013). 'ESSENTIA: an Audio Analysis Library for Music Information Retrieval.' International Society for Music Information Retrieval Conference (ISMIR'13). 493-498.


Object Description
------------------

- Creation argument: This is the threshold level for the onset detector. It will typically be in the range 10 and 50, depending on the source audio. With a higher threshold, the object will only report very prominent attacks, such as those produced by percussive instruments.

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

--- loudness: loudness of the extract.

--- mfcc: Mel Frecuency Cepstral Coefficients averaged over the full slice.

--- noisiness: a measure of the flatness in the spectrum. It can help determine whether a sound is pitched or un-pitched (i.e. noisy).

--- tempCentroid: Temporal centroid, reporting where the energy is concentrated in the analysed chunk. 
​
Methods: DelayMode. Once an onset is reported (on the 2nd outlet), a larger window is used to estimate some of other descriptors of the event, and then listed in outlet3. The window size over which these parameters are calculated can be set dynamically with the method "delayMode"  followed by a scalar value in ms. A delayMode time of 0 estimates over the full audio chunk between onset reports.


System Requirements
-------------------

Operating System:
Mac OS X (10.7 or newer), A recent version of Debian/Ubuntu, Microsoft Windows (7/8 32-bit or 64-bit)

Software:
Pd-extended (version 0.42.5 or newer) or Max (Version 5 or newer).

On Mac and Windows please make sure you use 32-bit versions of Pd and Max.

Please note: examples are only provided for Pd and not Max (save for a help file). Please consult the Pd patches and adopt them for your own
needs in Max.

Installation
------------

Linux:
=====

1) Install Dependencies.
sudo apt-get install build-essential libyaml-dev libfftw3-dev

2) Use the appropriate external (Only for 32-bit Linux).

​​If you're on 64-bit you don't have to do anything.
If you're on 32-bit, rename "essentiaRT~.pd_linux32" to "essentiaRT~.pd_linux"

OSX:
===

Add the essentiaRT~ folder to the Pd path (in Pd > Preferences) or copy the folder with the binaries, the examples and the utils where Pd can find it (typically to ~/Library/Pd or to the /extra folder inside Pd (Pd-Extended > Show Package Contents > Contents/Resources/extra).

The dependencies (fftw and libtag) should be contained within the external, but if there are any issues you can install them via

brew install fftw --universal
brew install libyaml --universal

Windows:
=======

For Pd - just run the examples from the folder, that should be it!

For Max - drop the 4 dll dependencies from win32 into C:\Program Files (x86)\Cycling '74\Max 6.1\support (Replacing Max 6.1 with the appropriate version number).

Acknowledgements
-----------------

EssentiaRT~ is built upon the work of Sebastian Böck (SuperFlux), Dmitry Bogdanov (Essentia) and the Pd and Pd-extended community. 

EssentiaRT~ was developed by Martin Hermant, Cárthach Ó Nuanáin and Ángel Faraldo within the Music Technology Group.

The related research was carried out in the frame of the GiantSteps project which is partly funded by the  European Commission (Seventh Framework Programme for research, technological development and demonstration under grant agreement no 610591) and the TECNIO network promoted by ACC1Ó agency by the Catalan Government.

© Copyright 2014 Music Technology Group, Universitat Pompeu Fabra. All Rights Reserved.
