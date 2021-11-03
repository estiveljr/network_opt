# Source: https://github.com/martinsch/pgmlink

# This module finds cplex.
#
# User can give CPLEX_ROOT_DIR as a hint stored in the cmake cache.
#
# It sets the following variables:
#  CPLEX_FOUND              - Set to false, or undefined, if cplex isn't found.
#  CPLEX_INCLUDE_DIRS       - include directory
#  CPLEX_LIBRARIES          - library files
include(CMakePrintHelpers)
INCLUDE(FindPackageHandleStandardArgs)



set(CPLEX_FOUND 1)
set(CPLEX_ROOT_DIR "C:/Program Files/IBM/ILOG/CPLEX_Studio201")

set(CPLEX_LIBRARY ${CPLEX_ROOT_DIR}/cplex/lib/x64_windows_msvc14/stat_mda/cplex2010.lib)
set(CPLEX_INCLUDE_DIR ${CPLEX_ROOT_DIR}/cplex/include)
set(CPLEX_ILOCPLEX_LIBRARY ${CPLEX_ROOT_DIR}/cplex/lib/x64_windows_msvc14/stat_mda/ilocplex.lib)
set(CPLEX_CONCERT_LIBRARY ${CPLEX_ROOT_DIR}/concert/lib/x64_windows_msvc14/stat_mda/concert.lib)
set(CPLEX_CONCERT_INCLUDE_DIR ${CPLEX_ROOT_DIR}/concert/include)

cmake_print_variables(CPLEX_LIBRARY)
cmake_print_variables(CPLEX_INCLUDE_DIR)
cmake_print_variables(CPLEX_ILOCPLEX_LIBRARY)
cmake_print_variables(CPLEX_CONCERT_LIBRARY)
cmake_print_variables(CPLEX_CONCERT_INCLUDE_DIR)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(CPLEX DEFAULT_MSG
        CPLEX_LIBRARY CPLEX_INCLUDE_DIR CPLEX_ILOCPLEX_LIBRARY CPLEX_CONCERT_LIBRARY CPLEX_CONCERT_INCLUDE_DIR)

IF(CPLEX_FOUND)
    SET(CPLEX_INCLUDE_DIRS ${CPLEX_INCLUDE_DIR} ${CPLEX_CONCERT_INCLUDE_DIR})
    SET(CPLEX_LIBRARIES ${CPLEX_CONCERT_LIBRARY} ${CPLEX_ILOCPLEX_LIBRARY} ${CPLEX_LIBRARY} )
    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        SET(CPLEX_LIBRARIES "${CPLEX_LIBRARIES};m;pthread")
    ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
ENDIF(CPLEX_FOUND)

MARK_AS_ADVANCED(CPLEX_LIBRARY CPLEX_INCLUDE_DIR CPLEX_ILOCPLEX_LIBRARY CPLEX_CONCERT_INCLUDE_DIR CPLEX_CONCERT_LIBRARY)