ACLOCAL_AMFLAGS = -I m4

#AUTOMAKE_OPTIONS = subdir-objects 1.6 dist-zip
AUTOMAKE_OPTIONS = subdir-objects 1.11 dist-zip dist-xz

SUBDIRS = .

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = $(OPUS_CFLAGS)

bin_PROGRAMS = vac-enc

noinst_HEADERS = src/wavreader.h

#dist_man_MANS = man/vac.1

vac_enc_SOURCES = src/main.c src/wavreader.c
vac_enc_CPPFLAGS = $(AM_CPPFLAGS)
vac_enc_CFLAGS = $(AM_CFLAGS) $(LIBOPUSENC_CFLAGS) $(SOXR_CFLAGS)
vac_enc_LDADD = $(LIBOPUSENC_LIBS) $(OPUS_LIBS) $(SOXR_LIBS) $(LIBM)
#vac_enc_MANS = man/vac.1

