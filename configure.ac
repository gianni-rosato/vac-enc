AC_INIT([vac-enc],[CURRENT_VERSION],[mr fan])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])

dnl enable silent rules on automake 1.11 and later
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_INIT_AUTOMAKE([1.11 foreign no-define])
AM_MAINTAINER_MODE([enable])

AC_CANONICAL_HOST
AM_PROG_CC_C_O

AC_PROG_CC_C99
AC_CHECK_HEADERS([inttypes.h alloca.h])
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE

LT_INIT

#Use a hacked up version of autoconf's AC_C_RESTRICT because it's not
#strong enough a test to detect old buggy versions of GCC (e.g. 2.95.3)
#Note: Both this and the test for variable-size arrays below are also
#      done by AC_PROG_CC_C99, but not thoroughly enough apparently.
AC_CACHE_CHECK([for C/C++ restrict keyword], ac_cv_c_restrict,
  [ac_cv_c_restrict=no
   # The order here caters to the fact that C++ does not require restrict.
   for ac_kw in __restrict __restrict__ _Restrict restrict; do
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
      [[typedef int * int_ptr;
        int foo (int_ptr $ac_kw ip, int * $ac_kw baz[]) {
        return ip[0];
       }]],
      [[int s[1];
        int * $ac_kw t = s;
        t[0] = 0;
        return foo(t, (void *)0)]])],
      [ac_cv_c_restrict=$ac_kw])
     test "$ac_cv_c_restrict" != no && break
   done
  ])

AH_VERBATIM([restrict],
[/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#undef restrict
/* Work around a bug in Sun C++: it does not support _Restrict or
   __restrict__, even though the corresponding Sun C compiler ends up with
   "#define restrict _Restrict" or "#define restrict __restrict__" in the
   previous line.  Perhaps some future version of Sun C++ will work with
   restrict; if so, hopefully it defines __RESTRICT like Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
# define __restrict__
#endif])

case $ac_cv_c_restrict in
   restrict) ;;
   no) AC_DEFINE([restrict], []) ;;
   *)  AC_DEFINE_UNQUOTED([restrict], [$ac_cv_c_restrict]) ;;
esac

AC_MSG_CHECKING(for C99 variable-size arrays)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
                   [[static int x; char a[++x]; a[sizeof a - 1] = 0; int N; return a[0];]])],
    [ has_var_arrays=yes
      AC_DEFINE([VAR_ARRAYS], [1], [Use C99 variable-size arrays])
    ],[
      has_var_arrays=no
    ])
AC_MSG_RESULT([$has_var_arrays])

if test "$CFLAGS" = "-g -O2"; then
    saved_CFLAGS="$CFLAGS"
    CFLAGS="-O3 -ffast-math"
    AC_MSG_CHECKING([if ${CC} supports -O3 -g -ffast-math])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[char foo;]])],
        [ AC_MSG_RESULT([yes])
          saved_CFLAGS="-O3 -g -ffast-math"
        ],[
          AC_MSG_RESULT([no])
        ])
    CFLAGS="$saved_CFLAGS"
fi

LT_LIB_M
AC_CHECK_LIB([winmm], [main])

on_x86=no
case "$host_cpu" in
i[[3456]]86 | x86_64)
  on_x86=yes
  ;;
esac

saved_LIBS="$LIBS"
LIBS="$LIBS $LIBM"
AC_CHECK_FUNCS([lrintf])
AC_CHECK_FUNCS([fminf])
AC_CHECK_FUNCS([fmaxf])
LIBS="$saved_LIBS"

AC_FUNC_FSEEKO
AC_CHECK_FUNCS([clock_gettime mach_absolute_time])
AC_CHECK_FUNCS([usleep nanosleep clock_nanosleep])

dnl check for pkg-config itself so we don't try the m4 macro without pkg-config
AC_CHECK_PROG(HAVE_PKG_CONFIG, pkg-config, yes)

dnl check for Opus
AS_IF([test "$HAVE_PKG_CONFIG" = "yes"],
 [PKG_CHECK_MODULES([OPUS],[opus >= 1.4])],
 [
  dnl fall back to the old school test
  XIPH_PATH_OPUS(, AC_MSG_ERROR([
    Opus is required to build this package!
    Please see https://opus-codec.org/ for how to obtain a copy.
  ]))
 ])

dnl check for libopusenc
AS_IF([test "$HAVE_PKG_CONFIG" = "yes"],
 [PKG_CHECK_MODULES([LIBOPUSENC],[libopusenc >= 0.2.1])],
 [
  dnl fall back to the old school test
  XIPH_PATH_LIBOPUSENC(, AC_MSG_ERROR([
    libopusenc is required to build this package!
    Please see https://opus-codec.org/ for how to obtain a copy.
  ]))
 ])

dnl check for soxr
AS_IF([test "$HAVE_PKG_CONFIG" = "yes"],
 [PKG_CHECK_MODULES([SOXR],[soxr >= 0.1.3])]
 )

on_windows=no
case "$host" in
*cygwin*|*mingw*)
  on_windows=yes
  ;;
esac

CFLAGS="$CFLAGS -W"

saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall -Wextra -Wcast-align -Wnested-externs -Wshadow -Wstrict-prototypes -Wno-sign-compare"
AC_MSG_CHECKING([if ${CC} supports -Wall -Wextra -Wcast-align -Wnested-externs -Wshadow -Wstrict-prototypes -Wno-sign-compare])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[char foo;]])],
    [ AC_MSG_RESULT([yes]) ],
    [ AC_MSG_RESULT([no])
      CFLAGS="$saved_CFLAGS"
    ])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([config.h])
AC_OUTPUT

AC_MSG_NOTICE([
------------------------------------------------------------------------
  $PACKAGE_NAME $PACKAGE_VERSION:  Automatic configuration OK.

    Compiler support:

      C99 var arrays: ................ ${has_var_arrays}
      C99 lrintf: .................... ${ac_cv_func_lrintf}

------------------------------------------------------------------------

 Type "make; make install" to compile and install
])
