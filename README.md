# eCheese61
Fun project to turn Raspberry Pi into "Transistor Organ From Hell"

# Motivation
Germanium transistors brought a revolution to organ design, replacing cogs and wheels with solid state dividers, driven from LC oscillators that "almost" stayed in tune. But, consumer electronics still had a long way to go with the idea of reliability, and playing a vintage instrument nowadays is out of reach for ordinary mortals and remains a gamble for the rest ("how many keys will break today?")

But, the sound... it has a very unique quality that is impossible to replicate with conventional sample-playback synths, at least for more than one note at a time.

These days, a Raspberry Pi has enough computational horsepower for the math, with some left to spare. Its reliability is head and shoulders above any piece of mid-60ies analog hardware, and adding a spare unit for USD 40 gets the hardware failure risk down into denormal number range. 

Also, the horrible PWM sound quality seems oddly appropriate (a circle driver for an external codec is available but so far untried).

# What?!
See for yourself: [Audio demo](transistorOrganFromHell.m4a)

This was phone-recorded though a Leslie cabinet. Background noises are me hacking away on a "Roland GO Piano" plastic keyboard. 

Note, the organ's key click itself seems pretty balanced.

# Requirements
* Raspberry Pi 3 (older versions should work too but need to recompile. The provided image is PI3-specific)
* USB MIDI class compliant keyboard (the vast majority of keyboards with USB connection for PC should work)
* Amplifier, guitar effect, etc. Plain headphones will work, too.
* SD card (this project uses the "circle" library without any further operating system)

# Features
Right now: 8' drawbar through the "reed" channel at fixed level. And that's all.

The organ is fully polyphonic with constant CPU load. Try this at your own risk.

Note, there is no USB hotplugging. A USB midi keyboard needs to be connected at boottime. Disconnection requires reboot.

# Performance
* Boot time: currently ~5 seconds from power-up, give or take some
* Latency / Jitter: 256 samples at 48 kHz => 6 ms plus USB

# Controls
MIDI USB noteon, noteoff

# Installation
The git repository contains the binary file kernel8-32.img (Raspberry PI 3 only). 

Copy to an otherwise bootable SD card (see circle instructions how to get the remaining files for a blank, FAT-formatted card).

Connect USB keyboard and power up. Audio is output from the 3.5 mm socket.

# Compilation
The project includes the circle library as submodule, which can be downloaded as follows:

cd eCheese61

git submodule update --init

See circle instructions on compiling (needs to know raspberry PI version and location of arm-none-eabi-xyz)

Then "make".

# Serial port upload (OPTIONAL!)
For repeated compilation, it is convenient (optional!) to install "bootloader7" on the card as kernel image:
https://github.com/dwelch67/raspberrypi/tree/master/boards/pi3/aarch32/bootloader07

Then use explicit "make kernel8-32.hex" and send it as plain text using 115200 baud (e.g. Teraterm, "File / send file"). On completed upload, press "g" to boot.

# Notes
Play through an amp and use tone controls (we wouldn't plug a vintage instrument straight to the console). This also softens the key click.

Right now, output volume is set to clip for more than one key at a time, which sounds quite OK (edit e.g. 1 => 0.1 in engine.c for a clean sound at lower volume)
