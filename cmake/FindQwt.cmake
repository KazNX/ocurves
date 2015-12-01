# Find Qt Widgets for Technical Applications (QWT)
# http://qwt.sourceforge.net/
#
# The find script defines the following variables:
# Qwt_FOUND - TRUE if QWT has been found.
# Qwt_INCLUDE_DIR - primary Qwt include directory.
# Qwt_INCLUDE_DIRS - all Qwt include directories.
# Qwt_LIBRARY - The primary QWT library found.
# Qwt_DEBUG_LIBRARY - (Optional) The QWT debug library, if found.
# Qwt_LIBRARY_DIR - The primary QWT library directory.
# Qwt_LIBRARIES - The full list of QWT libraries. This includes
#   seperate specification of debug libraries if available.
# Qwt_RUNTIME_LIBRARIES - (WIN32) For DLL platforms, set to the Qwt DLLs matching the libraries.
# Qwt_PDBS - (WIN32) For DLL platforms, set to the Qwt PDB files matching the libraries.
# Qwt_VERSION - The full version string resolved from qwt_global.h
# Qwt_MAJOR_VERSION - Major version number found.
# Qwt_MINOR_VERSION - Minor version number found.
# Qwt_PATCH_VERSION - Patch version number found.
#
# Note: The variable prefix may change depending on the find_package usage.
# The variables are actually defined with whatever package is requested of
# find_package(). Thus if find_package(QWT) is given, then the variables are
# all prefixed with "QWT_". Similarly, using find_package(Qwt) yields the
# prefix "Qwt_".

# Cache the requested package name so we can export variables with that string.
set(QWT ${CMAKE_FIND_PACKAGE_NAME})

# Find Qwt header.
find_path(${QWT}_INCLUDE_DIR NAMES qwt.h
          HINTS $ENV{${QWT}_DIR}/include $ENV{QWT_DIR} $ENV{Qwt_DIR}/include
                ${${QWT}_DIR}/include ${QWT_DIR}/include ${Qwt_DIR}/include
                $ENV{QTDIR}/include ${QT_INCLUDE_DIR}
          PATH_SUFFIXES qwt qwt-qt5)

find_library(${QWT}_DEBUG_LIBRARY NAMES qwtd qwt-qt5d
             HINTS $ENV{${QWT}_DIR}/lib $ENV{QWT_DIR}/lib $ENV{Qwt_DIR}/lib
                   ${${QWT}_DIR}/lib ${QWT_DIR}/lib ${Qwt_DIR}/lib
                   ${QT_LIBRARY_DIR}
              PATH_SUFFIXES qwt qwt-qt5)
find_library(${QWT}_LIBRARY NAMES qwt qwt-qt5
             HINTS $ENV{${QWT}_DIR}/lib $ENV{QWT_DIR}/lib $ENV{Qwt_DIR}/lib
                   ${${QWT}_DIR}/lib ${QWT_DIR}/lib ${Qwt_DIR}/lib
                   ${QT_LIBRARY_DIR}
              PATH_SUFFIXES qwt qwt-qt5)

set(${QWT}_INCLUDE_DIRS ${${QWT}_INCLUDE_DIR})

# Resolve version number.
set(QWT_GLOBAL ${${QWT}_INCLUDE_DIR}/qwt_global.h)

if(EXISTS ${QWT_GLOBAL})
  # Read version number
  file(STRINGS "${QWT_GLOBAL}" ${QWT}_VERSION REGEX "^[ \t]*\\#define[ \t]+QWT_VERSION_STR[ \t]+.*$")
  if(${QWT}_VERSION)
    # Extract just the version string.
    string(REGEX REPLACE ".*\\\"(.*)\\\".*"
           "\\1" ${QWT}_VERSION "${${QWT}_VERSION}")
    string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)"
           "\\1;\\2;\\3" QWT_VERSION_SPLIT "${${QWT}_VERSION}")
    list(GET QWT_VERSION_SPLIT 0 ${QWT}_MAJOR_VERSION)
    list(GET QWT_VERSION_SPLIT 1 ${QWT}_MINOR_VERSION)
    list(GET QWT_VERSION_SPLIT 2 ${QWT}_PATCH_VERSION)
  endif(${QWT}_VERSION)
endif(EXISTS ${QWT_GLOBAL})

# Check version
if(${QWT}_VERSION VERSION_LESS ${QWT}_FIND_VERSION)
  # Unsuitable version
  message(SEND_ERROR "Unsuitable Qwt version found: ${${QWT}_VERSION}. Required ${PACKAGE_FIND_VERSION}")
endif(${QWT}_VERSION VERSION_LESS ${QWT}_FIND_VERSION)

if(${QWT}_DEBUG_LIBRARY)
  set(${QWT}_LIBRARIES general ${${QWT}_LIBRARY} debug ${${QWT}_DEBUG_LIBRARY})
elseif(${QWT}_DEBUG_LIBRARY)
  set(${QWT}_LIBRARIES ${${QWT}_LIBRARY})
endif(${QWT}_DEBUG_LIBRARY)

# Find a companion for LIB.
# Search in LIB directory and SUBDIRS with recursion up each
# directory in the path of LIB, looking for a file with the
# extension EXT.
#
# Example:
#   find_companion_file(DLLFILE "${Qwt_LIBRARY}" DLL bin)
# Look for DLL for Qwt_LIBRARY, looking in the same directory,
# then up one and in a bin directory.
function(FIND_COMPANION_FILE VAR LIB TYPE SUBDIRS)
  get_filename_component(BASEDIR "${LIB}" DIRECTORY)
  get_filename_component(BASEFILE "${LIB}" NAME_WE)

  set(${VAR} ${VAR}-NOTFOUND PARENT_SCOPE)

  # Try the current directory.
  set(TEST_FILE "${BASEDIR}/${BASEFILE}.${TYPE}")
  if(EXISTS "${TEST_FILE}")
    set(${VAR} "${TEST_FILE}" PARENT_SCOPE)
    return()
  endif(EXISTS "${TEST_FILE}")

  set(LAST_BASE "")
  while(BASEDIR AND NOT BASEDIR STREQUAL LAST_BASE)
    foreach(SUB ${SUBDIRS})
      set(TEST_FILE "${BASEDIR}/${SUB}/${BASEFILE}.${TYPE}")
      if(EXISTS "${TEST_FILE}")
        set(${VAR} "${TEST_FILE}" PARENT_SCOPE)
        return()
      endif(EXISTS "${TEST_FILE}")
    endforeach(SUB)

    # Jump up a directory and recurse.
    set(LAST_BASE "${BASEDIR}")
    get_filename_component(BASEDIR "${BASEDIR}" DIRECTORY)
  endwhile(BASEDIR AND NOT BASEDIR STREQUAL LAST_BASE)
endfunction(FIND_COMPANION_FILE)

# Macro for resolving DLL and PDB files.
function(RESOLVE_LIBRARY_FILES VAR LIBRARIES TYPE)
  unset(${VAR})
  unset(PREFIX)
  foreach(LIB ${LIBRARIES})
    if(LIB MATCHES "debug|general|optimized")
      set(PREFIX ${LIB})
    else()
      find_companion_file(LIB_COMPANION "${LIB}" ${TYPE} "bin")
      if(LIB_COMPANION)
        if(DEFINED PREFIX)
          list(APPEND ${VAR} ${PREFIX})
        endif(DEFINED PREFIX)
        list(APPEND ${VAR} "${LIB_COMPANION}")
      endif(LIB_COMPANION)
      unset(PREFIX)
    endif()
  endforeach(LIB)

  set(${VAR} "${${VAR}}" PARENT_SCOPE)
endfunction(RESOLVE_LIBRARY_FILES)

if(WIN32)
  RESOLVE_LIBRARY_FILES(${QWT}_RUNTIME_LIBRARIES "${${QWT}_LIBRARIES}" "dll")
  RESOLVE_LIBRARY_FILES(${QWT}_PDBS "${${QWT}_LIBRARIES}" "pdb")
  set(${QWT}_RUNTIME_LIBRARIES "${${QWT}_RUNTIME_LIBRARIES}" CACHE PATH "Qwt binary DLLs")
  set(${QWT}_PDBS "${${QWT}_RUNTIME_LIBRARIES}" CACHE PATH "Qwt debug PDBs")
endif(WIN32)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${QWT}
    FOUND_VAR ${QWT}_FOUND
    REQUIRED_VARS ${QWT}_INCLUDE_DIR ${QWT}_INCLUDE_DIRS ${QWT}_LIBRARY ${QWT}_LIBRARIES
    VERSION_VAR ${QWT}_VERSION
    )

mark_as_advanced(
  ${QWT}_INCLUDE_DIR
  ${QWT}_INCLUDE_DIRS
  ${QWT}_LIBRARY
  ${QWT}_LIBRARIES
  ${QWT}_RUNTIME_LIBRARIES
  ${QWT}_PDBS
  ${QWT}_VERSION
  ${QWT}_MAJOR_VERSION
  ${QWT}_MINOR_VERSION
  ${QWT}_PATCH_VERSION
)

# message(STATUS "################################################################################")
# message(STATUS "${QWT}_INCLUDE_DIR ${${QWT}_INCLUDE_DIR}")
# message(STATUS "${QWT}_INCLUDE_DIRS ${${QWT}_INCLUDE_DIRS}")
# message(STATUS "${QWT}_LIBRARY ${${QWT}_LIBRARY}")
# message(STATUS "${QWT}_LIBRARIES ${${QWT}_LIBRARIES}")
# message(STATUS "${QWT}_RUNTIME_LIBRARIES ${${QWT}_RUNTIME_LIBRARIES}")
# message(STATUS "${QWT}_PDBS ${${QWT}_PDBS}")
# message(STATUS "${QWT}_VERSION ${${QWT}_VERSION}")
# message(STATUS "${QWT}_MAJOR_VERSION ${${QWT}_MAJOR_VERSION}")
# message(STATUS "${QWT}_MINOR_VERSION ${${QWT}_MINOR_VERSION}")
# message(STATUS "${QWT}_PATCH_VERSION ${${QWT}_PATCH_VERSION}")
# message(STATUS "################################################################################")
