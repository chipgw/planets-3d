# - Find NFD
# Find the NativeFileDialog includes and library
#
#   NFD_FOUND       - True if NFD found.
#   NFD_INCLUDE_DIR - where to find NFD.h, etc.
#   NFD_LIBRARIES   - List of libraries when using NFD.
#

IF( NFD_INCLUDE_DIR )
    # Already in cache, be silent
    SET( NFD_FIND_QUIETLY TRUE )
ENDIF( NFD_INCLUDE_DIR )

FIND_PATH( NFD_INCLUDE_DIR "nfd.h"
           PATH_SUFFIXES "NFD" )

FIND_LIBRARY( NFD_LIBRARIES
              NAMES "nfd"
              PATH_SUFFIXES "NFD" )

# handle the QUIETLY and REQUIRED arguments and set NFD_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( "NFD" DEFAULT_MSG NFD_INCLUDE_DIR NFD_LIBRARIES )

MARK_AS_ADVANCED( NFD_INCLUDE_DIR NFD_LIBRARIES )
