AC_INIT([xfce4-mxdock-plugin], [0.0.1])
AM_INIT_AUTOMAKE([1.15.0 -Wall -Werror])

AM_CONFIG_HEADER(config.h)

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
#IT_PROG_INTLTOOL([0.40.0])
AM_PROG_AR

dnl Vertical status bar option
AC_ARG_ENABLE([vertical_bar],
			  AS_HELP_STRING([--disable-vertical-bar], [Disable vertical bar]))

AS_IF([test "x$enable_vertical_bar" != "xno"], [
	dnl Doing stuff to enable feature
	AC_DEFINE([VERTICAL_BAR_ENABLED], [1], [Vertical Bar Status is Enabled])
])

dnl Initialize libtool, prevent static libraries to remove pointless
dnl compilation
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl Check for standard header files
AC_HEADER_STDC

AC_CHECK_FUNCS([bind_textdomain_codeset])

dnl Configure the panel plugin (XFCE4 xdt-depends.m4)
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-2.0], [4.12.0])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.12.0])

dnl Check for i18n support
XDT_I18N([@LINGUAS@])

dnl Checks for libraries
PKG_CHECK_MODULES([WNCK], [libwnck-3.0 >= 3.30.0])
PKG_CHECK_MODULES([CAIRO], [cairo >= 1.16.0])
PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.24.0])

AC_SUBST([WNCK_CFLAGS])
AC_SUBST([CAIRO_LIBS])
AC_SUBST([WNCK_LIBS])

AC_SUBST([GTK_CFLAGS])
AC_SUBST([CAIRO_LIBS])
AC_SUBST([GTK_LIBS])

dnl Substitute platform flags
AC_MSG_CHECKING([PLATFORM_CPPFLAGS])
AC_MSG_RESULT([$PLATFORM_CPPFLAGS])
AC_SUBST([PLATFORM_CPPFLAGS])
AC_MSG_CHECKING([PLATFORM_CFLAGS])
AC_MSG_RESULT([$PLATFORM_CFLAGS])
AC_SUBST([PLATFORM_CFLAGS])
AC_MSG_CHECKING([PLATFORM_LDFLAGS])
AC_MSG_RESULT([$PLATFORM_LDFLAGS])
AC_SUBST([PLATFORM_LDFLAGS])

_cppflags="${CPPFLAGS}"
_ldflags="${LDFLAGS}"
 
CPPFLAGS=${_cppflags}
LDFLAGS=${_ldflags}

dnl Create makefiles
AC_CONFIG_FILES([Makefile 
        src/Makefile
])
AC_OUTPUT
