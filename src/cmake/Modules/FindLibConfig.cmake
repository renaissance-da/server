# Locate libconfig, libconfig++
# This module defines
#  LIBCONFIG_FOUND
#  LIBCONFIG_LIBRARIES
#  LIBCONFIG_INCLUDE_DIR

# Search for includes

find_path(LIBCONFIG_INCLUDE_DIR
  NAMES
    libconfig.h++
  PATHS
    /usr/local/include
    /usr/include
    /opt/local/include
    /opt/csw/include
    /opt/include
    ${LIBCONFIG_DIR}/include
)

find_library(LIBCONFIG_LIBRARIES
  NAMES
    config++
    libconfig++
  PATHS
    /usr/local
    /usr/lib/x86_64-linux-gnu
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    ${LIBCONFIG_DIR}/lib
  NO_DEFAULT_PATH
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibConfig DEFAULT_MSG LIBCONFIG_LIBRARIES LIBCONFIG_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBCONFIG_INCLUDE_DIR LIBCONFIG_LIBRARIES)
