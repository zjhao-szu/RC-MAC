INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_RC_MAC RC_MAC)

FIND_PATH(
    RC_MAC_INCLUDE_DIRS
    NAMES RC_MAC/api.h
    HINTS $ENV{RC_MAC_DIR}/include
        ${PC_RC_MAC_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    RC_MAC_LIBRARIES
    NAMES gnuradio-RC_MAC
    HINTS $ENV{RC_MAC_DIR}/lib
        ${PC_RC_MAC_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/RC_MACTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RC_MAC DEFAULT_MSG RC_MAC_LIBRARIES RC_MAC_INCLUDE_DIRS)
MARK_AS_ADVANCED(RC_MAC_LIBRARIES RC_MAC_INCLUDE_DIRS)
