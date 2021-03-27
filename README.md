# FG-Roam - Standalone [Flightgear](http://www.flightgear.org) Terrain Viewer

![fg-roam screenshot][1]

This is mainly used a the "synthetic vision" part of
[SoFIS][2] an open-source EFIS.

## Dependencies

This project depends on:

* OpenGL 2.1 / GLES 2
* Glib (Only GArray and GPtrArray are really used/needed)
* SDL2
* SDL2_Image
* libcurl

## Supported Platforms

As part of Sofis, this terrain viewer as been tested on x86 and Raspberry Pi
B (the very first one). It should also work one the RPi 0 which has the same
hardware as the B+ with more RAM and a faster processor.

When building on the Pi, you might need to edit the Makefile to link against
libbrcmGLES.

## Building

There is currently no sophisticated build system that checks and detects everything
as needed. There is one simple Makefile that you can edit and use to set build parameters
if you need to.

```sh
$ git clone https://github.com/sam-itt/fg-roam.git
$ cd fg-roam
$ git submodule update --init --recursive
$ wget https://github.com/sam-itt/fg-roam/archive/media.tar.gz
$ tar -xf media.tar.gz --strip-components=1
$ cd src
$ make
$ ./view-gl
```

[1]: https://github.com/sam-itt/fg-roam/blob/media/fg-roam-screenshot.png?raw=true
[2]: https://github.com/sam-itt/sofis
