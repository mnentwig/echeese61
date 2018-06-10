# eCheese61
Fun project to turn Raspberry Pi into "Transistor Organ From Hell"

# Motivation
Germanium transistors brought a revolution to organ design, replacing cogs and wheels with solid state dividers, driven from LC oscillators that "almost" stayed in tune. But, consumer electronics still had a long way to go with the idea of reliability, and playing a vintage instrument nowadays is out of reach for ordinary mortals and remains a gamble for the rest ("how many keys will break today?")

But, the sound... it has a very unique quality that is impossible to replicate with conventional sample-playback synths because the relative phase between partials coming from the same oscillator is deterministic.

These days, a Raspberry Pi has enough computational horsepower for the math, with some left to spare. Its reliability is head and shoulders above any piece of mid-60ies analog hardware, and adding a spare unit for USD 40 gets the hardware failure risk down into denormal number range. 

Also, the horrible PWM sound quality seems oddly appropriate (a circle driver for an external codec is available but so far untried).

# Bare metal implementation
This project uses the "circle" library without any further operating system.

# Features
Right now: 8' drawbar through the "reed" channel at fixed level. And that's all.

The organ is fully polyphonic with constant CPU load. Try at your own risk.

# Controls
MIDI USB (must be visible at boot. No hotplug!

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
