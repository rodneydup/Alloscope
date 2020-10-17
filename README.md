# Alloscope

Alloscope is an X-Y or "octopus" configuration oscilloscope application for visualizing stereo audio files. It can be used to explore Lissajous curves, cymatic pattterns, and vector art that usually requires a vector display like an analog oscilloscope or Vectrex.

The app was built on the Allolib framework (https://github.com/AlloSphere-Research-Group/allolib).

## How to compile / run

On a bash shell:

    ./configure.sh

This will execute cmake on the project

    ./run.sh

This will compile the project, and run the binary if compilation is successful.

## How to perform a distclean
If you need to delete the distribution,

    ./distclean.sh

should recursively clean all the build directories of the project including those of allolib and its submodules.
