CLI-WabbitEmu README
====================

What is CLI-WabbitEmu?
----------------------
CLI-WabbitEmu is a TI-8x emulator that runs entirely in your terminal (i.e. it offers a "command line interface").
It is based on a decent TI-8x emulator, WabbitEmu.
Hence the name: CLI-WabbitEmu!

It is designed to run on any system which offers an ncurses interface (I expect the primary audience will be Linux users, but I'm primarily developing it on OS X).

CLI-WabbitEmu is currently in the very early stages of development, but I hope to achieve feature parity with other WabbitEmu distributions down the road. 

Where did it come from CLI-WabbitEmu?
-------------------------------------
This project is a fork of [Albert Huang's wxWabbitemu](https://github.com/alberthdev/wxwabbitemu), which in turn is a fork of the original [RevSoft WabbitEmu](https://wabbit.codeplex.com/).

Building CLI-WabbitEmu
---------------------
You need the following:
* A working C compiler. This includes, but is not limited to, GCC and Clang.
* ncurses, panel, form, and menu. These are all very low-level libraries and are likely already installed on your machine if you're running any sort of modern Unix-like OS. 
* CMake (at least version 2.8.11).
* An IDE or build system for CMake to generate project files for.
Once you have installed the dependencies, you can go ahead and start the build!

In a terminal type, navigate to the directory containing this README, then execute the following.
* `mkdir build && cd build`
* `cmake ..`
* `make`
If you wish to use a different project generator, e.g. XCode, pass the `-G` option to `cmake`, and the appropriate project generator name.

Other Stuff
-----------
That's it! We hope you enjoy CLI-WabbitEmu! Should it fail to do
something, or crash, please help us by reporting bugs in our 
issue tracker, located at [here](https://github.com/elfprince13/cli-wabbitemu/issues).
