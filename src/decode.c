/*
* vac-enc
* Copyright (C) 2024 Gianni Rosato
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "unicode_support_wrapper.h"

#include "decode.h"
#include "flac.h"
#include "wavreader.h"

#define OPUSENC_BUFFER_SAMPLES 96000
#define FLAC_BUFFER_EXTENSION  32768

int (*vac_get_samples)(FileInfo *, void *);
FILE *flac_input;
fx_flac_state_t flac_state;
uint32_t remaining_samples = 128; // Initially used for malloc and fread in vac_open_file()

static inline int16_t normalize_u8(unsigned char data)
{
    return (data ^ 0x80) << 8;
}

static inline int32_t normalize_s24le(unsigned char *data)
{
    return (data[2] << 24) | (data[1] << 16) | (data[0] << 8);
}

static int read_wav_u8(FileInfo *info, void *ibuf)
{
    int bytes_read = wav_read_data(info->in, (unsigned char *)ibuf+info->ilen*info->channels,
                                   info->ilen*info->channels);
    for (int i = 0; i < info->ilen*info->channels; i++) {
        *((int16_t *)ibuf+i) = normalize_u8(*((unsigned char *)ibuf+info->ilen*info->channels+i));
    }

    return bytes_read;
}

static int read_wav_s24le(FileInfo *info, void *ibuf)
{
    int bytes_read = wav_read_data(info->in, (unsigned char *)ibuf+info->ilen*info->channels,
                                   info->ilen*info->channels*3);
    for (int i = 0, j = 0; i < info->ilen*info->channels; i++, j += 3) {
        *((int32_t *)ibuf+i) = normalize_s24le((unsigned char *)ibuf+info->ilen*info->channels+j);
    }

    return bytes_read/3;
}

static int read_wav_normal(FileInfo *info, void *ibuf)
{
    return wav_read_data(info->in, ibuf, info->ilen*info->channels*info->bit_depth/8) >> info->shift;
}

static int read_flac_normal(FileInfo *info, void *ibuf)
{
    const int offset = info->ilen*info->channels; // For ibuf, also maximum samples per iteration
    uint8_t *const flac_buf = (uint8_t *)ibuf+offset*sizeof(int32_t); // Start of flac read buffer
    int samples = 0;
    int cur_read;
    static int prev_read = FLAC_BUFFER_EXTENSION;
    static uint32_t to_read = FLAC_BUFFER_EXTENSION;
    remaining_samples = offset;

    while (1) {
        cur_read = fread(flac_buf+prev_read-to_read, 1,
                         FLAC_BUFFER_EXTENSION-prev_read+to_read, flac_input);
        to_read = cur_read < FLAC_BUFFER_EXTENSION-prev_read+to_read ?
        prev_read-to_read+cur_read : FLAC_BUFFER_EXTENSION;
        prev_read = to_read;
        if (!to_read)
            break;

        fx_flac_process((fx_flac_t *)info->in, flac_buf, &to_read,
                        (int32_t *)ibuf+samples, &remaining_samples);

        memmove(flac_buf, flac_buf+to_read, prev_read-to_read); // Shift unread bytes to front

        samples += remaining_samples;
        remaining_samples = offset - samples;
        if (!remaining_samples)
            break;
    }

    return samples;
}

int vac_open_file(const char *infile, FileInfo *info, void **ibuf, void **obuf)
{
    info->in = wav_read_open(infile);
    if (!info->in) {
        fprintf(stderr, "Unable to open input file.\n");
        return 1;
    }

    if (!wav_get_header(info->in, &info->format, &info->channels,
                        &info->sample_rate, &info->bit_depth, &info->length)) {
        wav_read_close(info->in);

        goto flac; // Not wav, try flac
    }

    if (!info->format || !info->channels || !info->sample_rate || !info->bit_depth || !info->length) {
        fprintf(stderr, "Bad WAVE file.\n");
        return 1;
    }
    info->length /= info->bit_depth/8;

    if (info->format != 1 && info->format != 3) { // To-do: alaw and ulaw
        fprintf(stderr, "Only LPCM and floating-point samples are supported.\n");
        return 1;
    }

    switch (info->bit_depth) { // The function we will be looping
        case 8:
            vac_get_samples = &read_wav_u8;
            break;
        case 16:
            vac_get_samples = &read_wav_normal;
            info->shift     = 1;
            break;
        case 24:
            vac_get_samples = &read_wav_s24le;
            break;
        case 32:
            vac_get_samples = &read_wav_normal;
            info->shift     = 2;
            break;
        case 64:
            vac_get_samples = &read_wav_normal;
            info->shift     = 3;
            break;
        default:
            fprintf(stderr, "Something went wrong.\n");
            return 1;
    }

    info->ilen = OPUSENC_BUFFER_SAMPLES * info->sample_rate / 48000;
    info->olen = OPUSENC_BUFFER_SAMPLES;

    // For 8-bit and 24-bit sources, we need to convert to the next 2^n-bit
    vac_get_samples != &read_wav_normal ?
    (*ibuf = malloc(info->ilen*info->channels*(1+info->bit_depth/8))) :
    (*ibuf = malloc(info->ilen*info->channels*info->bit_depth/8));
    if (!*ibuf) {
        fprintf(stderr, "Unable to allocate sufficient memory.\n");
        return 1;
    }

    goto end;

flac:

    info->in = FX_FLAC_ALLOC_DEFAULT();
    *ibuf = malloc(remaining_samples); // Should be enough to parse the flac header
    if (!info->in || !*ibuf) {
        fprintf(stderr, "Unable to allocate sufficient memory.\n");
        return 1;
    }

    flac_input = fopen_utf8(infile, "rb");
    remaining_samples = fread(*ibuf, 1, remaining_samples, flac_input);
    flac_state = fx_flac_process((fx_flac_t *)info->in, *ibuf, &remaining_samples, NULL, NULL);
    if (!flac_state) { // Not flac either, fail
        fprintf(stderr, "Invalid input file.\n");
        return 1;
    }

    info->sample_rate = fx_flac_get_streaminfo((fx_flac_t *)info->in, FLAC_KEY_SAMPLE_RATE);
    info->channels    = fx_flac_get_streaminfo((fx_flac_t *)info->in, FLAC_KEY_N_CHANNELS);
    info->bit_depth   = fx_flac_get_streaminfo((fx_flac_t *)info->in, FLAC_KEY_SAMPLE_SIZE);
    info->length      = fx_flac_get_streaminfo((fx_flac_t *)info->in, FLAC_KEY_N_SAMPLES) * info->channels;
    info->format      = 0; // Signal flac input

    if (!info->channels || !info->sample_rate || !info->bit_depth || !info->length) {
        fprintf(stderr, "Bad FLAC file.\n");
        return 1;
    }

    fseek(flac_input, 0, SEEK_SET); // Reset file pointer for decoding
    fx_flac_reset(info->in);

    vac_get_samples = &read_flac_normal;

    info->ilen = OPUSENC_BUFFER_SAMPLES * info->sample_rate / 48000;
    info->olen = OPUSENC_BUFFER_SAMPLES;

    *ibuf = realloc(*ibuf, info->ilen*info->channels*sizeof(int32_t)+FLAC_BUFFER_EXTENSION);
    if (!*ibuf) {
        fprintf(stderr, "Unable to allocate sufficient memory.\n");
        return 1;
    }

end:

    *obuf = malloc(info->olen*info->channels*sizeof(float)); // For ope_encoder_write_float()
    if (!*obuf) {
        fprintf(stderr, "Unable to allocate sufficient memory.\n");
        return 1;
    }

    return 0;
}

void vac_close_file(void *in, int format)
{
    format ? wav_read_close(in) : free(in);
}