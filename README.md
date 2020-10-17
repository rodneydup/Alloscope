# Alloscope

Alloscope is an X-Y or "octopus" configuration oscilloscope application for visualizing stereo audio files. It can be used to explore Lissajous curves and vector art that usually requires a vector display like an analog oscilloscope or Vectrex. This software aims to mimic these analog machines as much as possible, but on a pixel-based display like your computer screen.

Alloscope can be compiled on Linux, OS X, or Windows. It is compatible with Jack or Soundflower, which allows the user to route audio from another application directly into Alloscope.

Alloscope accepts OSC control to modulate the parameters of color and thickness.

The app was built using the Allolib framework (https://github.com/AlloSphere-Research-Group/allolib).

## Usage

After launching the app, press 'g' on your keyboard to open the GUI control panel. Here you can cusomtize everything from the look of the scope to the audio settings. Press 'g' again to hide the gui.

There are two modes on the Oscilloscope. The primary mode is "external," in which the scope drawn is derived from audio piped into the app. The other mode is "internal" and is just included for testing purposes and experimentation. In internal mode, a sine tone is generated at a frequency determined by the "Base Frequency" control and a second sine tone is generated relative to that frequency based on the "Ratio" control. You can use this mode to draw basic Lissajous figures.

### OSC
Send osc to address 127.0.0.1 and port 12000. At the moment, two parameters can be controlled via OSC - Hue and Thickness.

#### Hue
To control the hue of the scope via OSC, send the argument "/hue <val>" where <val> is a floating point number between 0.0 and 1.0. You *can* send values outside of this range, but they will be wrapped around (modulo 1).

#### Thickness
To control the thickness of the scope via OSC, send the argument "/thicc <val>" where <val> is a floating point number between 0.0 and 1.0. Values outside of this range are clipped, rather than wrapped.

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
