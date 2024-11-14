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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_MSC_VER)
# include <getopt.h>
#else
# include <unistd.h>
#endif

#include "unicode_support_wrapper.h"

#include <opusenc.h>
#include <soxr.h>

#include "decode.h"
#include "version.h"

typedef struct SoxBlock {
    soxr_t resampler;
    soxr_error_t soxerr;
    soxr_io_spec_t io;
} SoxBlock;

typedef struct OpusBlock {
    OggOpusEnc *enc;
    OggOpusComments *comments;
    int opusencerr;
} OpusBlock;

int init_resampler(FileInfo info, SoxBlock *sb)
{
    soxr_quality_spec_t quality = { // Resampler quality settings
        .precision      = 33,
        .phase_response = 50,
        .passband_end   = 0.913,
        .stopband_begin = 1,
        .e              = NULL,
        .flags          = SOXR_ROLLOFF_NONE | SOXR_HI_PREC_CLOCK
    };

    switch (info.bit_depth) {
        case 8:
        case 16:
            sb->io = soxr_io_spec(SOXR_INT16_I, SOXR_FLOAT32_I);
            break;
        case 24:
        case 32:
            sb->io = (info.format == 1) ? soxr_io_spec(SOXR_INT32_I, SOXR_FLOAT32_I) :
                                          soxr_io_spec(SOXR_FLOAT32_I, SOXR_FLOAT32_I);
            break;
        case 64:
            if (info.format == 1) {
                fprintf(stderr, "64-bit LPCM is unsupported. Use float instead.\n");
                return 1;
            } else {
                sb->io = soxr_io_spec(SOXR_FLOAT64_I, SOXR_FLOAT32_I);
            }
            break;
        default:
            fprintf(stderr, "Unsupported word length: %d\n", info.bit_depth);
            return 1;
    }
    if (!info.format) // FLAC override
        sb->io = soxr_io_spec(SOXR_INT32_I, SOXR_FLOAT32_I);

    sb->resampler = soxr_create(info.sample_rate, 48000, info.channels,
                                &sb->soxerr, &sb->io, &quality, NULL);
    if (!sb->resampler) {
        fprintf(stderr, "Error initializing soxr: %s\n", sb->soxerr);
        return 1;
    }

    return 0;
}

int init_encoder(const char *outfile, FileInfo info, OpusBlock *ob, opus_int32 *bitrate,
                        int have_bitrate, opus_int32 *lsb, int have_lsb, int vbr_mode, int *mapping)
{
    if (info.channels > 2)
        *mapping = 1;

    ob->comments = ope_comments_create();
    ope_comments_add(ob->comments, "encoder", "vac-enc");
    ob->enc = ope_encoder_create_file(outfile, ob->comments, 48000,
                                      info.channels, *mapping, &ob->opusencerr);
    if (!ob->enc) {
        fprintf(stderr, "Cannot write to output file: %s\n", ope_strerror(ob->opusencerr));
        return 1;
    }

    if (!have_bitrate)
        *bitrate = 1000*(64+32*info.channels);
    if (*bitrate > 256000*info.channels || *bitrate < 6000) {
        fprintf(stderr, "Bitrate out of range 6-%d kbps.\n", info.channels*256);
        return 1;
    }
    ope_encoder_ctl(ob->enc, OPUS_SET_BITRATE(*bitrate));
    if (vbr_mode > 2 || vbr_mode < 0) {
        fprintf(stderr, "VBR mode must be 0 (CBR), 1 (CVBR), or 2 (VBR).\n");
        return 1;
    }
    if (!vbr_mode) ope_encoder_ctl(ob->enc, OPUS_SET_VBR(0));
    if (vbr_mode > 1) ope_encoder_ctl(ob->enc, OPUS_SET_VBR_CONSTRAINT(0));
    ope_encoder_ctl(ob->enc, OPE_SET_COMMENT_PADDING(0));
    ope_encoder_ctl(ob->enc, OPE_SET_MUXING_DELAY(0));
    if (!have_lsb) *lsb = info.bit_depth > 24 ? 24 : info.bit_depth;
    if (*lsb > 24 || *lsb < 8) {
        fprintf(stderr, "LSB out of range 8-24.\n");
        return 1;
    }
    ope_encoder_ctl(ob->enc, OPUS_SET_LSB_DEPTH(*lsb));

    return 0;
}

void usage(const char *path)
{
    fprintf(stderr, "vac-enc %s (using %s, %s, libsoxr %s)\n",
            VAC_VERSION, opus_get_version_string(), ope_get_version_string(), SOXR_THIS_VERSION_STR);
    fprintf(stderr, "Usage: %s [-b kbps] <WAVE/FLAC input> <Ogg Opus output>\n", path);
}

int main(int argc, char **argv)
{
    int ret;
    int ch;
    opus_int32 bitrate;
    opus_int32 lsb;
    int have_bitrate = 0;
    int have_lsb = 0;
    int vbr_mode = 2;
    int mapping = 0;
    FileInfo info;
    SoxBlock sb;
    OpusBlock ob;
    size_t idone, odone;
    void *ibuf, *obuf;
    clock_t start, end;
#ifdef WIN_UNICODE
    int argc_utf8;
    char **argv_utf8;

    (void)argc;
    (void)argv;
    init_commandline_arguments_utf8(&argc_utf8, &argv_utf8);
#endif

    while ((ch = getopt(argc_utf8, argv_utf8, "b:l:v:")) != -1) {
        switch (ch) {
            case 'b':
                bitrate = (opus_int32)(atof(optarg)*1000);
                have_bitrate = 1;
                break;
            case 'l':
                lsb = atoi(optarg);
                have_lsb = 1;
                break;
            case 'v':
                vbr_mode = atoi(optarg);
                break;
            case '?':
            default:
                usage(argv_utf8[0]);
                return 1;
        }
    }
    if (argc_utf8 - optind < 2) {
        usage(argv_utf8[0]);
        return 1;
    }
    if (!strcmp(argv_utf8[argc_utf8-1], "-") || !strcmp(argv_utf8[argc_utf8-2], "-")) {
        fprintf(stderr, "stdin/stdout is not supported.\n");
        return 1;
    }
    if (!strcmp(argv_utf8[argc_utf8-1], argv_utf8[argc_utf8-2])) {
        fprintf(stderr, "Input and output file cannot be the same.\n");
        return 1;
    }

    ret = vac_open_file(argv_utf8[argc_utf8-2], &info, &ibuf, &obuf);
    if (ret)
        return 1;

    ret = init_resampler(info, &sb);
    if (ret)
        return 1;

    ret = init_encoder(argv_utf8[argc_utf8-1], info, &ob, &bitrate,
                       have_bitrate, &lsb, have_lsb, vbr_mode, &mapping);
    if (ret)
        return 1;

    fprintf(stderr, "\n\tEncoding library  ::  %s\n", opus_get_version_string());
    fprintf(stderr, "\n\tTarget bitrate    ::  %.3f kbps (%s)\n", (float)bitrate/1000,
            vbr_mode < 2 ? (vbr_mode < 1 ? "CBR" : "CVBR") : "VBR");
    fprintf(stderr, "\n\tSample rate       ::  ");
    if (info.sample_rate != 48000) fprintf(stderr, "%.1f kHz -> ", (float)info.sample_rate/1000);
    fprintf(stderr, "48.0 kHz\n\n");

    start = clock();

    while (1) { // Main encoding loop, maximum two seconds of 48 kHz audio per iteration
        int samples;
        static int tot_samples = 0;
        static char *progress_bar[26] = {
            "[                         ]", "[=                        ]",
            "[==                       ]", "[===                      ]",
            "[====                     ]", "[=====                    ]",
            "[======                   ]", "[=======                  ]",
            "[========                 ]", "[=========                ]",
            "[==========               ]", "[===========              ]",
            "[============             ]", "[=============            ]",
            "[==============           ]", "[===============          ]",
            "[================         ]", "[=================        ]",
            "[==================       ]", "[===================      ]",
            "[====================     ]", "[=====================    ]",
            "[======================   ]", "[=======================  ]",
            "[======================== ]", "[=========================]"
        };

        samples = (*vac_get_samples)(&info, ibuf);
        soxr_process(sb.resampler, ibuf, samples/info.channels, &idone, obuf, info.olen, &odone);
        ope_encoder_write_float(ob.enc, obuf, odone);

        end = clock();
        tot_samples += samples;
        fprintf(stderr, "\r\tProcessing %s %3.0f%%, %3.fx realtime",
                progress_bar[25*tot_samples/info.length], 99.99*tot_samples/info.length,
                (float)tot_samples*CLOCKS_PER_SEC/((float)info.sample_rate*info.channels*(end-start)));

        if (samples < info.ilen*info.channels)
            break;
    }
    soxr_process(sb.resampler, NULL, 1, &idone, obuf, info.ilen+info.olen, &odone);
    ope_encoder_write_float(ob.enc, obuf, odone); // Dirty hack to pad last frame

    end = clock();
    fprintf(stderr, "\r\tProcessing [=========================] 100%%, %3.fx realtime\n",
           (float)info.length*CLOCKS_PER_SEC/((float)info.channels*info.sample_rate*(end-start)));
#ifndef WIN_UNICODE // Because Windows terminal will do it regardless
    fprintf(stderr, "\n");
#endif

    ope_encoder_drain(ob.enc);
    ope_encoder_destroy(ob.enc);
    ope_comments_destroy(ob.comments);
    soxr_delete(sb.resampler);
    free(obuf); free(ibuf);
    vac_close_file(info.in, info.format);
#ifdef WIN_UNICODE
    free_commandline_arguments_utf8(&argc_utf8, &argv_utf8);
#endif

    return 0;
}