
[essentiaRT~]

bugs/issues:

- There is a problem when loading on the last Pd-extended (v. 0.43.4; Tcl v. 8.5.11). It seems that is related to TCL code. In my computer (angel), it loads the 1st time I open the patch after booting (and then I need to restart computer!). Then, when I try to modify it (change the threshold, for example) it will freeze the GUI for the object. 

However, it seems to run fine in in Pd-extended (v. 0.42.5, tcl v. 8.4.19). In general, I have been experiencing problems with GUI object, such canvas, view on parents and the like in the newest pd-extended (0.43.4)