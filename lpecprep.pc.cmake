prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@PKG_CONFIG_LIBDIR@
includedir=@INCLUDE_INSTALL_DIR@

Name: lpecprep
Description: LPecPrep periodic error correction curve creator
URL: https://github.com/rlancaste/stellarsolver
Version: @LPecPrep_VERSION@
Libs: -L${libdir} -llpecprep
Libs.private: -lpthread
Cflags: -I${includedir}

