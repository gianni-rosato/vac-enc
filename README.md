# vac-enc

_Fast and simple Opus command-line frontend_

## Build instructions

There are two build systems available; CMake and Zig.

### CMake

1. Before building, ensure you have the `libsoxr`, `libopus`, and `libopusenc` libraries installed, as well as `cmake` version 3.10 or newer. `vac-enc` uses the pkg-config utility to find these libraries, so confirm they are installed and the `PKG_CONFIG_PATH` environment variable is set correctly.

2. Clone the repository and navigate to the `build` directory.

    ```bash
    git clone https://github.com/gianni-rosato/vac-enc.git
    cd vac-enc/build
    ```

3. From here, you can invoke `cmake ..` as you wish, or use the shell script provided.

    ```bash
    ./build.sh
    ```

The `vac-enc` binary will be located in `bin`. From there, you can move it to a location in your `PATH` (like /usr/local/bin) or use it directly.

### Zig

1. Before building, ensure you have the `libsoxr`, `libopus`, and `libopusenc` libraries installed, as well as `zig` version 0.11.0 or newer. `vac-enc` uses the pkg-config utility to find these libraries, so confirm they are installed and the `PKG_CONFIG_PATH` environment variable is set correctly.

2. Clone the repository and navigate to the root directory.

    ```bash
    git clone https://github.com/gianni-rosato/vac-enc.git
    cd vac-enc/
    ```

3. Build the project.

    ```bash
    zig build
    ```

The `vac-enc` binary will be located in `zig-out/bin`. Fome there, you can move it to a location in your `PATH` (like /usr/local/bin) or use it directly.

## Usage

`vac-enc` is simple to use. Running it with no arguments yields useful information.

```bash
vac-enc 0.2 (using libopus 1.5.2, libopusenc 0.2.1, libsoxr 0.1.3)
Usage: ./vac-enc [-b kbps] <WAVE/FLAC input> <Ogg Opus output>
```

A sane bitrate will be chosen if not specified, or you can provide your own.

```bash
./vac-enc -b64 my-song.flac test.opus
```

## Extras

Also included is the `vac-auto` script, which can convert from various filetypes with FFmpeg.

## License

This project is licensed under the GPL-3.0 License -- see the [LICENSE](LICENSE) file for details.

## Acknowledgments

-   wavreader source files by Martin Storsjö
-   libfoxenflac source files by Andreas Stöckel 

