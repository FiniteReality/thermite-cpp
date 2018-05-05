# Thermite #

A C++14 voice library for Discord, using uWebSockets and libuv.

## NOTE ##

This is mostly for experimental purposes. If you're looking for a more mature
voice backend, try looking at [lavaplayer] and its related project, [lavalink]!

## Building ##

```sh
$ git clone https://github.com/FiniteReality/thermite.git
$ cd thermite
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Options ###

- `THERMITE_DEBUG`: Enable debug logging (default: ON)
- `THERMITE_BUILD_SAMPLES`: Build samples (default: ON)
- `THERMITE_BUILD_SAMPLE_<<NAME>>`: Build thermite sample `NAME` (default: ON)

### Samples ###

- `cli`: Simple command-line example using discord.py as a frontend

[lavaplayer]: https://github.com/sedmelluq/lavaplayer
[lavalink]: https://github.com/Frederikam/Lavalink
