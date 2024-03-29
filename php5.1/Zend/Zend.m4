dnl
dnl $Id$
dnl
dnl This file contains Zend specific autoconf functions.
dnl

AC_DEFUN([LIBZEND_CHECK_INT_TYPE],[
AC_MSG_CHECKING(for $1)
AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H  
#include <sys/types.h>
#endif
#if HAVE_INTTYPES_H  
#include <inttypes.h>
#elif HAVE_STDINT_H
#include <stdint.h>
#endif],
[if (($1 *) 0)
  return 0;
if (sizeof ($1))
  return 0;
],[
  AC_DEFINE_UNQUOTED([HAVE_]translit($1,a-z_-,A-Z__), 1,[Define if $1 type is present. ])
  AC_MSG_RESULT(yes)
], AC_MSG_RESULT(no)
)dnl
])

AC_DEFUN([LIBZEND_BASIC_CHECKS],[

AC_REQUIRE([AC_PROG_YACC])
AC_REQUIRE([AC_PROG_CC])
AC_REQUIRE([AC_PROG_CC_C_O])
AC_REQUIRE([AC_PROG_LEX])
AC_REQUIRE([AC_HEADER_STDC])

LIBZEND_BISON_CHECK

dnl Ugly hack to get around a problem with gcc on AIX.
if test "$CC" = "gcc" -a "$ac_cv_prog_cc_g" = "yes" -a \
   "`uname -sv`" = "AIX 4"; then
	CFLAGS=`echo $CFLAGS | sed -e 's/-g//'`
fi

dnl Hack to work around a Mac OS X cpp problem
dnl Known versions needing this workaround are 5.3 and 5.4
if test "$ac_cv_prog_gcc" = "yes" -a "`uname -s`" = "Rhapsody"; then
        CPPFLAGS="$CPPFLAGS -traditional-cpp"
fi

AC_CHECK_HEADERS(
inttypes.h \
stdint.h \
limits.h \
malloc.h \
string.h \
unistd.h \
stdarg.h \
sys/types.h \
sys/time.h \
signal.h \
unix.h \
stdlib.h \
mach-o/dyld.h \
dlfcn.h)

AC_TYPE_SIZE_T
AC_TYPE_SIGNAL

AC_DEFUN([LIBZEND_LIBDL_CHECKS],[
AC_CHECK_LIB(dl, dlopen, [LIBS="-ldl $LIBS"])
AC_CHECK_FUNC(dlopen,[AC_DEFINE(HAVE_LIBDL, 1,[ ])])
])

AC_DEFUN([LIBZEND_DLSYM_CHECK],[
dnl
dnl Ugly hack to check if dlsym() requires a leading underscore in symbol name.
dnl
AC_MSG_CHECKING([whether dlsym() requires a leading underscore in symbol names])
_LT_AC_TRY_DLOPEN_SELF([
  AC_MSG_RESULT(no)
], [
  AC_MSG_RESULT(yes)
  AC_DEFINE(DLSYM_NEEDS_UNDERSCORE, 1, [Define if dlsym() requires a leading underscore in symbol names. ])
], [
  AC_MSG_RESULT(no)
], [])
])

dnl This is required for QNX and may be some BSD derived systems
AC_CHECK_TYPE( uint, unsigned int )
AC_CHECK_TYPE( ulong, unsigned long )

dnl Check if int32_t and uint32_t are defined
LIBZEND_CHECK_INT_TYPE(int32_t)
LIBZEND_CHECK_INT_TYPE(uint32_t)

dnl Checks for library functions.
AC_FUNC_VPRINTF
AC_FUNC_MEMCMP
AC_FUNC_ALLOCA
AC_CHECK_FUNCS(memcpy strdup getpid kill strtod strtol finite fpclass)
AC_ZEND_BROKEN_SPRINTF

AC_CHECK_FUNCS(finite isfinite isinf isnan)

ZEND_FP_EXCEPT
	
])

AC_DEFUN([LIBZEND_ENABLE_DEBUG],[

AC_ARG_ENABLE(debug,
[  --enable-debug          Compile with debugging symbols],[
  ZEND_DEBUG=$enableval
],[
  ZEND_DEBUG=no
])  

])

AC_DEFUN([LIBZEND_OTHER_CHECKS],[

AC_ARG_WITH(zend-vm,
[  --with-zend-vm=TYPE     Set virtual machine dispatch method. Type is
                          one of "CALL", "SWITCH" or "GOTO" [TYPE=CALL]],[
  PHP_ZEND_VM=$withval
],[
  PHP_ZEND_VM=CALL
])

AC_ARG_ENABLE(zend-memory-manager,
[  --disable-zend-memory-manager
                          Disable the Zend memory manager - FOR DEVELOPERS ONLY!!],
[
  ZEND_USE_ZEND_ALLOC=$enableval
], [
  ZEND_USE_ZEND_ALLOC=yes
])

AC_ARG_ENABLE(maintainer-zts,
[  --enable-maintainer-zts Enable thread safety - for code maintainers only!!],[
  ZEND_MAINTAINER_ZTS=$enableval
],[
  ZEND_MAINTAINER_ZTS=no
])  

AC_ARG_ENABLE(inline-optimization,
[  --disable-inline-optimization 
                          If building zend_execute.lo fails, try this switch],[
  ZEND_INLINE_OPTIMIZATION=$enableval
],[
  ZEND_INLINE_OPTIMIZATION=yes
])

AC_ARG_ENABLE(memory-limit,
[  --enable-memory-limit   Compile with memory limit support], [
  ZEND_MEMORY_LIMIT=$enableval
],[
  ZEND_MEMORY_LIMIT=no
])

AC_ARG_ENABLE(zend-multibyte,
[  --enable-zend-multibyte Compile with zend multibyte support], [
  ZEND_MULTIBYTE=$enableval
],[
  ZEND_MULTIBYTE=no
])

AC_MSG_CHECKING([virtual machine dispatch method])
AC_MSG_RESULT($PHP_ZEND_VM)

AC_MSG_CHECKING(whether to enable the Zend memory manager)
AC_MSG_RESULT($ZEND_USE_ZEND_ALLOC)

AC_MSG_CHECKING(whether to enable thread-safety)
AC_MSG_RESULT($ZEND_MAINTAINER_ZTS)

AC_MSG_CHECKING(whether to enable inline optimization for GCC)
AC_MSG_RESULT($ZEND_INLINE_OPTIMIZATION)

AC_MSG_CHECKING(whether to enable a memory limit)
AC_MSG_RESULT($ZEND_MEMORY_LIMIT)

AC_MSG_CHECKING(whether to enable Zend debugging)
AC_MSG_RESULT($ZEND_DEBUG)

AC_MSG_CHECKING(whether to enable Zend multibyte)
AC_MSG_RESULT($ZEND_MULTIBYTE)
	
case $PHP_ZEND_VM in
  SWITCH)
    AC_DEFINE(ZEND_VM_KIND,ZEND_VM_KIND_SWITCH,[virtual machine dispatch method])
    ;;
  GOTO)
    AC_DEFINE(ZEND_VM_KIND,ZEND_VM_KIND_GOTO,[virtual machine dispatch method])
    ;;
  *)
    PHP_ZEND_VM=CALL
    AC_DEFINE(ZEND_VM_KIND,ZEND_VM_KIND_CALL,[virtual machine dispatch method])
    ;;
esac

if test "$ZEND_DEBUG" = "yes"; then
  AC_DEFINE(ZEND_DEBUG,1,[ ])
  echo " $CFLAGS" | grep ' -g' >/dev/null || DEBUG_CFLAGS="-g"
  if test "$CFLAGS" = "-g -O2"; then
  	CFLAGS=-g
  fi
  test -n "$GCC" && DEBUG_CFLAGS="$DEBUG_CFLAGS -Wall"
  test -n "$GCC" && test "$USE_MAINTAINER_MODE" = "yes" && \
    DEBUG_CFLAGS="$DEBUG_CFLAGS -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations"
else
  AC_DEFINE(ZEND_DEBUG,0,[ ])
fi

test -n "$DEBUG_CFLAGS" && CFLAGS="$CFLAGS $DEBUG_CFLAGS"

if test "$ZEND_USE_ZEND_ALLOC" = "yes"; then
  AC_DEFINE(USE_ZEND_ALLOC,1,[Use Zend memory manager])
else
  AC_DEFINE(USE_ZEND_ALLOC,0,[Use Zend memory manager])
fi

if test "$ZEND_MAINTAINER_ZTS" = "yes"; then
  AC_DEFINE(ZTS,1,[ ])
  CFLAGS="$CFLAGS -DZTS"
  LIBZEND_CPLUSPLUS_CHECKS
fi  

if test "$ZEND_MEMORY_LIMIT" = "yes"; then
  AC_DEFINE(MEMORY_LIMIT, 1, [Memory limit])
else
  AC_DEFINE(MEMORY_LIMIT, 0, [Memory limit])
fi

if test "$ZEND_MULTIBYTE" = "yes"; then
  AC_DEFINE(ZEND_MULTIBYTE, 1, [ ])
fi

changequote({,})
if test -n "$GCC" && test "$ZEND_INLINE_OPTIMIZATION" != "yes"; then
  INLINE_CFLAGS=`echo $ac_n "$CFLAGS $ac_c" | sed s/-O[0-9s]*//`
else
  INLINE_CFLAGS="$CFLAGS"
fi
changequote([,])

AC_C_INLINE

AC_SUBST(INLINE_CFLAGS)

AC_MSG_CHECKING(target system is Darwin)
if echo "$target" | grep "darwin" > /dev/null; then
  AC_DEFINE([DARWIN], 1, [Define if the target system is darwin])
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

dnl test and set the alignment define for ZEND_MM
dnl this also does the logarithmic test for ZEND_MM.
AC_MSG_CHECKING(for MM alignment and log values)

AC_TRY_RUN([
#include <stdio.h>

typedef union _mm_align_test {
  void *ptr;
  double dbl;
  long lng;
} mm_align_test;

#if (defined (__GNUC__) && __GNUC__ >= 2)
#define ZEND_MM_ALIGNMENT (__alignof__ (mm_align_test))
#else
#define ZEND_MM_ALIGNMENT (sizeof(mm_align_test))
#endif

int main()
{
  int i = ZEND_MM_ALIGNMENT;
  int zeros = 0;
  FILE *fp;

  while (i & ~0x1) {
    zeros++;
    i = i >> 1;
  }

  fp = fopen("conftest.zend", "w");
  fprintf(fp, "%d %d\n", ZEND_MM_ALIGNMENT, zeros);  
  fclose(fp);

  exit(0);
}
], [
  LIBZEND_MM_ALIGN=`cat conftest.zend | cut -d ' ' -f 1`
  LIBZEND_MM_ALIGN_LOG2=`cat conftest.zend | cut -d ' ' -f 2`
  AC_DEFINE_UNQUOTED(ZEND_MM_ALIGNMENT, $LIBZEND_MM_ALIGN, [ ])
  AC_DEFINE_UNQUOTED(ZEND_MM_ALIGNMENT_LOG2, $LIBZEND_MM_ALIGN_LOG2, [ ]) 
], [], [
  dnl cross-compile needs something here
  LIBZEND_MM_ALIGN=8
])

AC_MSG_RESULT(done)

])


AC_DEFUN([LIBZEND_CPLUSPLUS_CHECKS],[

])

