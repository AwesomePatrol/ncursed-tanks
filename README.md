ncursed-tanks
=============

ncursed-tanks is a net-based multiplayer game written in C using ncurses interface.
This solution gives a user capability to run it on all *nix platforms.

Usage
-----

./menu
#run config creator/modifier in user-friendly mode
#config is saved in "server.conf", which is automatically read by server

./client SERVER_IP NAME
#run game's client and connect it to server specified by SERVER_IP, sets NAME

./server
#run server which hosts the game, parameters are specified in "server.conf"

Gameplay
--------
Gameplay is explained in 'doc/game'.

Demo
----
![ncursed-tanks GIF demo](http://patrol.neutrino.re/repo/ncursed-tanks.gif)

Dependencies
------------
##make
> Build managemen tool
https://www.gnu.org/software/make/

##ncurses
> Free software emulation of curses
http://www.gnu.org/software/ncurses/ncurses.html

##POSIX
> system support for POSIX
http://www.unix.org/

Optional dependencies
---------------------
##doxygen
> Standard tool for generating documentation
http://www.stack.nl/~dimitri/doxygen/

Build
-----
run "make" to build whole project
run "make clean" to remove build files
run "make doc" to generate documentation
