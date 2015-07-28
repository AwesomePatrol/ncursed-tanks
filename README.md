ncursed-tanks
=============

ncursed-tanks is a net-based multiplayer game written in C using ncurses interface.
This solution gives a user capability to run it on all *nix platforms.


Usage
-----

```bash
> ./config_menu
## run config creator/modifier in user-friendly mode
## config is saved in "server.conf", which is read by the server

> ./client SERVER_IP NICKNAME
## run game's client and connect it to the server specified by SERVER_IP, using NICKNAME

> ./server
## run the server which hosts the game, parameters are specified in "server.conf"
```


Gameplay
--------
Gameplay is explained in `doc/game`.


Demo
----
![ncursed-tanks GIF demo](http://patrol.neutrino.re/repo/ncursed-tanks.gif)


Dependencies
------------
* [make](https://www.gnu.org/software/make/) - Build management tool
* [ncurses](http://www.gnu.org/software/ncurses/ncurses.html) - Free software emulation of curses
* system support for [POSIX](http://www.unix.org/)
* [doxygen](http://www.stack.nl/~dimitri/doxygen/) (optional) - Standard tool for generating documentation


Build
-----
* run `make` to build the whole project
* run `make clean` to remove build files
* run `make doc` to generate documentation
