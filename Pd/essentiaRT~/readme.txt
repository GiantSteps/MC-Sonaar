bugs/issues:

Pd-version
———————————

—FIXED——FIXED——FIXED——FIXED——FIXED——FIXED——FIXED——FIXED——FIXED
There is a problem when loading on the last Pd-extended (v. 0.43.4; Tcl v. 8.5.11). It seems that is related to TCL code. In my computer (angel), it loads the 1st time I open the patch after booting (and then I need to restart computer!). Then, when I try to modify it (change the threshold, for example) it will freeze the GUI for the object. However, it seems to run better in in Pd-extended (v. 0.42.5, tcl v. 8.4.19), so I am testing in that environment temporarily. In general, I have been experiencing problems with GUI object, such as canvas, view-on-parent and the like in the newest pd-extended (0.43.4).
—FIXED——FIXED——FIXED——FIXED——FIXED——FIXED——FIXED——FIXED——FIXED

Performance
———————————

Check /PD/essentiaRT~/essentiaRT~_comparison.pd for performance details. Roughly, different instances yield different results and the performance of the onset detector is about 50% with very simple test samples.


Blocksize??
—————————

A weird behaviour: essentiaRT~ will not work in my computer when connecting a signal directly to its input. A pair of send~/receive~ needs to be inserted in-between.


===================================== essentiaRT~ ==================================

Description
————————————

argument: threshold level for the Super Flux algorithm.

inlet_#1: audio signal

outlet_#1: onset strength value at audio rate on the LAST SAMPLE OF THE PREVIOUS AUDIO BLOCK. I think it should be on the first sample of the block. It can be temporarily solved with a 1-sample delay.

outlet_#2: list of features at control rate:

    - onset_strength: same as in outlet1, at control rate.
    
    - int.tri: 139-dimensional vector, containing what????

outlet_#3: list of higher-level features. These values are estimated over a bigger window (100 ms??) once an onset is detected:

    - nsdf.mean: mean of the normalised squared difference function over the analysis window.

    - nsdf.var: and its variance.

    - pitched.mean: this gives the mean of the 'pitchniness' of the sound by measuring the peakiness in the spectrum.

    - pitched.var: and its variance.


Possible Todo’s (besides  debugging)
————————————————————————————————————
 
- It would be nice to be able to change the threshold of the super flux algorithm with a method/message like “threshold $1”

- And give the possibility to change the size of the higher-level feature extraction window.

- Avoid the ‘list’ header in outlets 2 and 3, for ease of access to the parameters.