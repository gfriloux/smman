m4_define([v_maj], [1])
m4_define([v_min], [0])
m4_define([v_mic], [1])
m4_define([v_ver], [v_maj.v_min.v_mic])


AC_INIT([Smman], [v_ver], [guillaume.friloux@aspfrance.com])

AC_PREREQ(2.01)
AC_CONFIG_SRCDIR([configure.ac])

AM_INIT_AUTOMAKE([1.6 dist-bzip2])

define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])

dnl we just have set the version info, then:
version_number=v_ver
AC_SUBST(version_number)

GCC_STACK_PROTECT_LIB
GCC_STACK_PROTECT_CC
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_MKDIR_P

AC_CONFIG_FILES([
Makefile
doc/Makefile
src/Makefile
src/bin/Makefile
])


PKG_CHECK_MODULES([eina], [eina], [have_eina="yes"], [have_eina="no"])
PKG_CHECK_MODULES([ecore], [ecore ecore-file], [have_ecore="yes"], [have_ecore="no"])
PKG_CHECK_MODULES([libcurl], [libcurl], [have_curl="yes"], [have_curl="no"])
AC_OUTPUT

echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "  eina.........: ${have_eina}"
echo "  ecore........: ${have_ecore}"
echo "  curl.........: ${have_curl}"
echo "  prefix.......: $prefix"
echo
echo "Build Options:"
echo "  CPPFLAGS.....: $CPPFLAGS"
echo "  CFLAGS.......: $CFLAGS"
echo "  LDFLAGS......: $LDFLAGS"
echo

