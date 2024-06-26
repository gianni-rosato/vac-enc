// Copyright (c) 2023 Gianni Rosato
// Gifted by mr fan

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opusenc.h>
#include <soxr.h>

#include "wavreader.h"

#define BUFFER_SAMPLES 96000

typedef struct {
  int Major;
  int Minor;
  int Patch;
} semver_t;

const semver_t VAC_SEMVER = {0, 0, 1};

int main(int argc, char **argv) {
  void *wav;
  int format, sample_rate, channels, word_length;

  soxr_t resampler;
  soxr_error_t soxerr;
  soxr_io_spec_t io;
  soxr_quality_spec_t quality;

  OggOpusEnc *enc;
  OggOpusComments *comments;
  int opusencerr;
  const char *opusver = opus_get_version_string();
  const char *opusencver = ope_get_version_string();

  if (argc != 4) {
    if (argc < 2) {
      printf("\x1b[36mvac-enc\x1b[0m v%i.%i.%i\n", VAC_SEMVER.Major,
             VAC_SEMVER.Minor, VAC_SEMVER.Patch);
      printf("Usage: %s [input.wav] [output.ogg] [kb/s]\n", argv[0]);
      return 1;
    }
    if (!strcmp("-h", argv[1]) || !strcmp("--help", argv[1])) {
      printf("\x1b[36mvac-enc\x1b[0m v%i.%i.%i | Opus encoding using the SoX "
             "resampler\n",
             VAC_SEMVER.Major, VAC_SEMVER.Minor, VAC_SEMVER.Patch);
      printf("\x1b[4mLibrary Versions\x1b[0m:\n");
      printf("\tOpus: %s\n", opusver);
      printf("\tOpusenc: %s\n", opusencver);
      printf("\tSoXR: libsoxr %s\n", SOXR_THIS_VERSION_STR);
      printf("\x1b[4mUsage\x1b[0m: %s [16-bit Little Endian WAVE input] "
             "[output.ogg] [kb/s]\n",
             argv[0]);
      // printf("\x1b[4mOptions\x1b[0m:\n");
      return 1;
    } else if (!strcmp("-v", argv[1]) || !strcmp("--version", argv[1])) {
      printf("\x1b[36mvac-enc\x1b[0m v%i.%i.%i |", VAC_SEMVER.Major,
             VAC_SEMVER.Minor, VAC_SEMVER.Patch);
      printf(" %s |", opusver);
      printf(" %s |", opusencver);
      printf(" libsoxr %s\n", SOXR_THIS_VERSION_STR);
      printf("Maintained by Gianni Rosato & mrfan\n");
      return 1;
    }
  }
  char *infile = argv[1];
  wav = wav_read_open(infile);
  if (!wav) {
    fprintf(stderr, "Unable to open input file\n");
    return 1;
  }
  if (!wav_get_header(wav, &format, &channels, &sample_rate, &word_length,
                      NULL)) {
    fprintf(stderr, "Bad wav file\n");
    return 1;
  }
  if (format != 1 && format != 3) {
    fprintf(stderr, "Only LPCM and floating-point samples are supported\n");
    return 1;
  }
  switch (word_length) {
  case 16:
    io = soxr_io_spec(SOXR_INT16_I, SOXR_FLOAT32_I);
    break;
  case 32:
    if (format == 1)
      io = soxr_io_spec(SOXR_INT32_I, SOXR_FLOAT32_I);
    else
      io = soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I);
    break;
  case 64:
    if (format == 1) {
      fprintf(stderr, "64-bit LPCM is unsupported; use float instead\n");
      return 1;
    } else
      io = soxr_io_spec(SOXR_FLOAT64_I, SOXR_FLOAT32_I);
    break;
  default:
    fprintf(stderr, "Unsupported word length: %d\n", word_length);
    return 1;
  }

  quality.precision = 33;
  quality.phase_response = 50;
  quality.passband_end = 0.913;
  quality.stopband_begin = 1;
  quality.e = 0;
  quality.flags = SOXR_ROLLOFF_NONE;
  resampler =
      soxr_create(sample_rate, 48000, channels, &soxerr, &io, &quality, NULL);
  if (!resampler) {
    fprintf(stderr, "Error initializing soxr: %s\n", soxerr);
    return 1;
  }

  int mapping = 0;
  if (channels > 2) {
    // mapping = 1;
    // fprintf(stderr, "Warning: surround audio must be mapped manually\n");
    fprintf(
        stderr,
        "Surround encoding disabled for this build due to mapping issues\n");
    return 1;
  }
  comments = ope_comments_create();
  ope_comments_add(comments, "encoder", "vac-enc");
  enc = ope_encoder_create_file(argv[2], comments, 48000, channels, mapping,
                                &opusencerr);
  if (!enc) {
    fprintf(stderr, "Cannot write to output file: %s\n",
            ope_strerror(opusencerr));
    ope_comments_destroy(comments);
    soxr_delete(resampler);
    wav_read_close(wav);
    return 1;
  }
  opusencerr =
      ope_encoder_ctl(enc, OPUS_SET_BITRATE((int)(atof(argv[3]) * 1000)));
  opusencerr = ope_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(0));
  opusencerr = ope_encoder_ctl(enc, OPE_SET_COMMENT_PADDING(0));
  if (word_length < 24)
    opusencerr = ope_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(word_length));

  const int bytes_per_sample = word_length / 8;
  const size_t ilen = BUFFER_SAMPLES / 48000 * sample_rate;
  const size_t olen = BUFFER_SAMPLES;
  size_t idone, odone;
  void *ibuf = malloc(ilen * channels * bytes_per_sample);
  void *obuf = malloc(olen * channels * 4); // Always float output
  unsigned int read;
  do {
    read = wav_read_data(wav, ibuf, ilen * channels * bytes_per_sample);
    soxerr = soxr_process(resampler, ibuf, read / channels / bytes_per_sample,
                          &idone, obuf, olen, &odone);
    opusencerr = ope_encoder_write_float(enc, obuf, odone);
  } while (read > 0);
  soxerr = soxr_process(resampler, NULL, 1, &idone, obuf, ilen + olen,
                        &odone); // Fully drain soxr buffer
  opusencerr = ope_encoder_write_float(enc, obuf, odone);

  ope_encoder_drain(enc);
  ope_encoder_destroy(enc);
  ope_comments_destroy(comments);
  free(obuf);
  free(ibuf);
  soxr_delete(resampler);
  wav_read_close(wav);

  return 0;
}
