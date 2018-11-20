# Thermite #

A C++17 voice framework for Discord, using Boost.Asio and Microsoft's
cpprestsdk.

Feel free to join my Discord server: https://discord.gg/Y4d9ZWJ

## NOTE ##

Thermite is for *experimental* purposes only!

Furthermore, Thermite is not even alpha software yet. If you're looking for a
more mature project, try looking at [lavaplayer] and its related project,
[lavalink].

## Platform Support ##

Right now, libthermite *should* work on all platforms, but semantics may behave
differently. See the [issues] for a list of issues relating to platform
support.

## Dependencies ##

- TBB
- libsodium
- libopus
- cpprestsdk
- Boost
- OpenSSL

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

None, yet!

[lavaplayer]: https://github.com/sedmelluq/lavaplayer
[lavalink]: https://github.com/Frederikam/Lavalink
[issues]: https://github.com/FiniteReality/thermite/issues
