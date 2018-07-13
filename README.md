# Thermite #
[![Discord][badge]][discord]

A C++14/C++17 voice framework for Discord, using µWebSockets and libuv.

## NOTE ##

Thermite is for *experimental* purposes only!

Furthermore, Thermite is not even alpha software yet. If you're looking for a
more mature project, try looking at [lavaplayer] and its related project,
[lavalink].

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

- `cli` - Small command-line sample using Discord.py rewrite and subprocess

## TODO ##

- CI support
- Code cleanup
- Consider Boost.Asio rewrite

[badge]: https://discordapp.com/api/guilds/446775596614418432/widget.png
[discord]: https://discord.gg/86GFdMY
[lavaplayer]: https://github.com/sedmelluq/lavaplayer
[lavalink]: https://github.com/Frederikam/Lavalink
