# vac-enc

_Opus encoding using the SoX resampler_

The Opus audio format only supports a few sample rates, which can limit its coding efficiency if resampling is not performed correctly. While many resampling programs are effective, the SoX resampler is considered to be the best. This program uses the SoX resampler to convert audio to a supported sample rate for Opus encoding.

```bash
vac-enc v0.0.X | Opus encoding using the SoX resampler
Library Versions:
	Opus: libopus 1.4
	Opusenc: libopusenc 0.2.1
	SOXR: libsoxr 0.1.3
Usage: vac-enc [16-bit Little Endian WAVE input] [output.ogg] [kb/s]
```

## Build instructions

There are two build systems present for building vac-enc; Zig and cmake. We recommend using Zig, but cmake instructions will be provided in the next section.

### Zig

0. Before building, make sure you have the `libsoxr`, `libopus`, and `libopusenc` libraries installed, as well as `zig` at least version 0.11.0. `vac-enc` uses the pkg-config utility to find these libraries, so make sure they are installed and the `PKG_CONFIG_PATH` environment variable is set correctly.

1. Clone the repository and navigate to the cloned directory.

```bash
git clone https://github.com/gianni-rosato/vac-enc.git
cd vac-enc/
```

2. Build the project.

```bash
zig build
```

The `vac-enc` binary will be located in `zig-out/bin/`. Fome there, you can move it to a location in your `PATH` (like /usr/local/bin) or use it directly.

### CMake

0. Before building, make sure you have the `libsoxr`, `libopus`, and `libopusenc` libraries installed, as well as `cmake` at least version 3.16. `vac-enc` uses the pkg-config utility to find these libraries, so make sure they are installed and the `PKG_CONFIG_PATH` environment variable is set correctly.

1. Clone the repository and navigate to the cloned directory.

```bash
git clone https://github.com/gianni-rosato/vac-enc.git
cd vac-enc/
```

2. Build the project using the script in `build/build.sh`.

```bash
cd build/
./build.sh
```

The `vac-enc` binary will be located in `build/`. From there, you can move it to a location in your `PATH` (like /usr/local/bin) or use it directly.

## Usage

`vac-enc` is a command-line program that takes three arguments: the input file, the output file, and the bitrate. The input file must be a 16-bit little-endian WAVE file, and the output file must be an Ogg file. The bitrate is specified in kilobits per second.

```bash title="Encode input.wav to output.ogg at 128 kb/s"
vac-enc input.wav output.ogg 128
```

To create valid WAVE input files, you can use FFmpeg like so:

```bash title="Convert input to valid WAV"
ffmpeg -i input.flac -c:a pcm_s16le output.wav
```

## Extras

Also included is the `vac-auto` script, which can convert from various filetypes with FFmpeg.

## License

This project is licensed under the GPL-3.0 License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

-   wavreader source is Copyright (C) 2009 Martin Storsjo
