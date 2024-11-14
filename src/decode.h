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

#ifndef VAC_DECODE_H
#define VAC_DECODE_H

typedef struct FileInfo {
    void *in;
    int format;
    int channels;
    int sample_rate;
    int bit_depth;
    unsigned int length; // Total samples
    int shift;
    size_t ilen;
    size_t olen;
} FileInfo;

extern int (*vac_get_samples)(FileInfo *, void *);

int vac_open_file(const char *infile, FileInfo *info, void **ibuf, void **obuf);

void vac_close_file(void *in, int format);

#endif