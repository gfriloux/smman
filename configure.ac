m4_define([v_rev], m4_esyscmd([(svnversion "." | grep -v exported || echo 0) | tr -d ' MSP\n']))

AC_INIT([Smman], [1.0], [guillaume.friloux@asp64.com])

AC_PREREQ(2.01)
AC_CONFIG_SRCDIR([configure.ac])

AM_INIT_AUTOMAKE([1.6 dist-bzip2])

define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])

dnl we just have set the version info, then:
version_number=1.0
revision_number="v_rev"
AC_SUBST(version_number)
AC_SUBST(revision_number)

AC_PROG_CC

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
echo
echo "  prefix.......: $prefix"
echo

