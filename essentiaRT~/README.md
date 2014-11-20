FFTW example written using flext. Below are instructions for installing flext on Mavericks.

INSTALL FLEXT
============

Easiest way is the command line, there is an Xcode project but the command line is easier and
installs everything neatly into /usr/local

Depending you might want to install PD, Max or both. 

1) PD (You need PD installed)
- Run:
sudo bash build.sh pd gcc

- This creates a file
./buildsys/config-mac-pd-gcc.txt now edit this file and add the appropriate PD location, in my case it's:-
"/Applications/Pd-extended.app/Contents/Resources"

- Run the script again to build it (you may need to run it twice to pick up the changes):
sudo bash build.sh pd gcc

- Hopefully it will compile correctly with no errors, now install it
sudo bash build.sh pd gcc install

2) Max (You need the Max SDK MAX5!!!!! (Still works with Max6)
- Download the Max SDK and copy the following frameworks to system locations as follows
"MaxAPI.framework" -> /Library/Frameworks
"MaxAudioAPI.framework" -> /Library/Frameworks

- Run:
sudo bash build.sh max gcc

- This creates a file
./buildsys/config-mac-max-gcc.txt now edit this file and add the appropriate Max SDK location, in my case it's:-
MAXSDKPATH=/Users/carthach/Dev/git/max6-sdk/c74support

- Run the script again to build it (you may need to run it twice to pick up the changes):
sudo bash build.sh max gcc

- Hopefully it will compile correctly with no errors, now install it
sudo bash build.sh max gcc install
