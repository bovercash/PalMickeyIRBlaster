# Pal Mickey IR Blaster
Pal Mickey IR Blaster

#### Description

This project goal is to document and record all Pal Mickey interactions for upload to an internet archival location.  

While many IR interactions still exist at Disney World, they are slowly disappearing.  If you would like to participate in finding and documenting codes please let us know!

We are stil in the process of determining the best way to document everything but feel free to reach out and we'll do our best!


####  Quick Start with Arduino UNO
For anyone just hoping to get their Pal Mickey talking, check out the **[Step-by-Step Quickstart Guide](./quickstart/README.md)** for wiring diagrams, software setup and how to get starting sending codes.


#### Recording Pal Mickey
The easiest and least impactful way to get a high-quality recording of Pal Mickey is to wire a line-out to the speaker output wires.  There are many "Line-Out to Mic-In" devices designed for recordign audio from cameras.  Search for a "Line-Out Attenuator" or similar device.

#### Handheld IR Blaster Project

This project is a stand-alone IR Blaster featuring a screen and input encoder with a 3D printed housing that runs on a 9volt battery.

This project uses two ATMEGA328 chips with one acting as the interface for adjusting the code to send and the other acting as the timer for transmitting the code.

See the `pal_mickey_ir_blaster.svg` file for a schematic and BOM.  The encoder and IR LEDs is 

###### Controls
Depressing the encoder toggles the input locaton XX to YY.  Holding the encoder button down for three seconds changes to Park-Change mode and allows you to change the selected park.  The selected part will display at the top of the display as a two-character abreviation.
