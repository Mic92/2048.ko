2048.ko - Play 2048 in the linux kernel
=======================================

This linux kernel module adds a new entry /proc/play_2048 in procfs, in order to
play the famous game [2048](http://git.io/2048)

![2048.ko](http://i.imgur.com/JLLzMi0.png)

INSTALL
=======

- tested with linux 3.14.+ (probably works on older kernel, if it compiles)
- requirements:
  - linux kernel headers (of your currently installed kernel)
  - gcc + make
- simply run:

         $ make

- and:

         $ insmod 2048.ko

USAGE
=====

To play the game just use this nifty snippet:

    sudo bash -c 'clear; while true; do cat /proc/play_2048; sleep 0.1; done& while IFS= read -r -n1 c; do echo $c > /proc/play_2048; done'

Cancel with Ctrl-C.

Use arrow keys or wasd or hlkl to move the tiles.
Restart with 'r'.

CONTRIBUTION
============

Cudos to mevdschee, who built original C-Version: [2048.c](https://github.com/mevdschee/2048.c), which I ported to the kernel.



