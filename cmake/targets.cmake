#==============================================================================
# This is the main script to include for defining a build target that uses the
# Common Software Development Framework.
#
# Author: Kazys Stepanas June 2013
# Copyright CSIRO 2013
#
# Targets are defined by starting the target definition using target_begin()
# then using various target_xxx() functions and closing the definition with
# target_end(). Target properties and dependencies may then be modified using
# standard CMake functions.
#==============================================================================

# Global definitions.
#cmake_minimum_required(VERSION 3.0)
cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0020 OLD)

include(CMakeParseArguments)
include(utility)

set(TARGET_SUFFIX_Debug "d")
set(TARGET_SUFFIX_MinSizeRel "m")
set(TARGET_SUFFIX_Release "")
set(TARGET_SUFFIX_RelWithDebInfo "i")

# Define BUILD build types.
set(BUILD_CONFIGS "Debug;MinSizeRel;Release;RelWithDebInfo")

#==============================================================================
# target_set_output_directories([TARGET] [MULTI] [PREFIX <prefix-dir>] [SUFFIX <suffix-dir>])
#
# Sets an output directory structure for build items for the current scope.
# This can be used at the project level or at the target level.
#
# All output directories lie under the CMAKE_BINARY_DIR with the following
# default structure:
# - /lib    - static library files (.lib, .a).
# - /bin    - executables and share library files (.exe, .dll, .so, .dylib)
#
# If multi-build configurations are supported then these are also applied
# as a suffix to the directories above. That is, binaries are built to:
# - /bin/Debug
# - /bin/MinSizeRel
# - /bin/Release
# - /bin/RelWithDebInfo
# This behaviour can be enabled by adding the MULTI argument.
#
# If TARGET is specified, then the current CMAKE_CURRENT_BINARY_DIR is used
# as the base directory instead of CMAKE_BINARY_DIR. This effectively generates
# a similar structure under the current target's directory.
#
# SUFFIX allows for a subdirectory suffix structure under the default
# structure. This is applied second, for example:
#   target_set_output_directories(SUFFIX plugins)
# Results in output to the following directories:
# - /lib/plugins
# - /bin/plugins
#==============================================================================
function(target_set_output_directories)
  CMAKE_PARSE_ARGUMENTS(TOD "TARGET;MULTI" "PREFIX;SUFFIX" "" "${ARGN}")

  if(TOD_TARGET)
    set(BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  else(TOD_TARGET)
    set(BASE_DIR "${CMAKE_BINARY_DIR}")
  endif(TOD_TARGET)

  # Ensure leading directory separator in prefix.
  if(TOD_PREFIX)
    set(TOD_PREFIX "/${TOD_PREFIX}")
  endif(TOD_PREFIX)

  # Ensure leading directory separator in suffix.
  if(TOD_SUFFIX)
    set(TOD_SUFFIX "/${TOD_SUFFIX}")
  endif(TOD_SUFFIX)

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BASE_DIR}${TOD_PREFIX}/lib${TOD_SUFFIX}" PARENT_SCOPE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BASE_DIR}${TOD_PREFIX}/bin${TOD_SUFFIX}" PARENT_SCOPE)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BASE_DIR}${TOD_PREFIX}/bin${TOD_SUFFIX}" PARENT_SCOPE)
  if(TOD_MULTI AND NOT CMAKE_BUILD_TYPE)
    # Don't use CMAKE_CONFIGURATION_TYPES as both it and CMAKE_BUILD_TYPE may be empty.
    # Don't use per config directories if CMAKE_BUILD_TYPE is set (only one directory.)
    foreach(CONF ${BUILD_CONFIGS})
      string(TOUPPER ${CONF} CONFU)
      set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFU} "${BASE_DIR}${TOD_PREFIX}/lib/${CONF}${TOD_SUFFIX}" PARENT_SCOPE)
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFU} "${BASE_DIR}${TOD_PREFIX}/bin/${CONF}${TOD_SUFFIX}" PARENT_SCOPE)
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFU} "${BASE_DIR}${TOD_PREFIX}/bin/${CONF}${TOD_SUFFIX}" PARENT_SCOPE)
    endforeach(CONF)
  else(TOD_MULTI AND NOT CMAKE_BUILD_TYPE)
    # Not using per configuration output directories. We need to explicitly set the common directory.
    foreach(CONF ${BUILD_CONFIGS})
      string(TOUPPER ${CONF} CONFU)
      set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFU} "${BASE_DIR}${TOD_PREFIX}/lib${TOD_SUFFIX}" PARENT_SCOPE)
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFU} "${BASE_DIR}${TOD_PREFIX}/bin${TOD_SUFFIX}" PARENT_SCOPE)
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFU} "${BASE_DIR}${TOD_PREFIX}/bin${TOD_SUFFIX}" PARENT_SCOPE)
    endforeach(CONF)
  endif(TOD_MULTI AND NOT CMAKE_BUILD_TYPE)
endfunction(target_set_output_directories)

#==============================================================================
# include_packages(package1 package2 ...)
#
# Attempt to add listed package include directories.
#
# This function handles various inconsistencies in how find_package() may define
# the includes directories for a package. The function handles for:
# - Where the package is actually a target name, either imported or locally
#   defined, using the target property INCLUDE_DIRECTORIES.
# - <PACKAGE>_INCLUDE_DIR
# - <PACKAGE>_INCLUDE_DIRS
#
# Where the package is not a target, each of the last two which exist are used.
#
# Iterates ${ARGN} for a list of packages adding include directories for each.
# Include directories are added using include_directories() passing 
# <package>_INCLUDE_DIR or <package>_INCLUDE_DIRS.
#==============================================================================
macro(include_packages)
  foreach(PACKAGE ${ARGN})
    if(TARGET ${PACKAGE})
      get_target_property(_inc_dirs ${PACKAGE} INCLUDE_DIRECTORIES)
      if(_inc_dirs)
        include_directories(${_inc_dirs})
      endif(_inc_dirs)
      # message("Include directories for target: ${PACKAGE}: ${_inc_dirs}")
    else(TARGET ${PACKAGE})
      if(DEFINED ${PACKAGE}_INCLUDE_DIR)
        include_directories(${${PACKAGE}_INCLUDE_DIR})
      endif()
      if(DEFINED ${PACKAGE}_INCLUDE_DIRS)
        include_directories(${${PACKAGE}_INCLUDE_DIRS})
      endif()
    endif(TARGET ${PACKAGE})
  endforeach(PACKAGE)
endmacro(include_packages)


#==============================================================================
# package_libraries(VAR PACKAGE TYPE)
#
# List the libraries for a PACKAGE using particular common find_package patters.
# A call to this function caters for the following patterns:
# - <PACKAGE>_<TYPE>_LIBRARY
# - <PACKAGE>_<TYPE>_LIBRARIES
# - <PACKAGE>_LIBRARY
# - <PACKAGE>_LIBRARIES
# The last two are only used when TYPE is an empty string.
#
# The contents of the <PACKAGE>_<TYPE>_LIBRARY are copied into VAR.
#==============================================================================
function(package_libraries VAR PACKAGE TYPE)
  set(_libs)
  if(NOT TYPE STREQUAL "")
    if(DEFINED ${PACKAGE}_${TYPE}_LIBRARY AND ${PACKAGE}_${TYPE}_LIBRARY)
      list(APPEND _libs "${${PACKAGE}_${TYPE}_LIBRARY}")
    endif(DEFINED ${PACKAGE}_${TYPE}_LIBRARY AND ${PACKAGE}_${TYPE}_LIBRARY)
    if(DEFINED ${PACKAGE}_${TYPE}_LIBRARIES AND ${PACKAGE}_${TYPE}_LIBRARIES)
      foreach(_lib ${${PACKAGE}_${TYPE}_LIBRARIES})
        list(APPEND _libs "${_lib}")
      endforeach(_lib)
    endif(DEFINED ${PACKAGE}_${TYPE}_LIBRARIES AND ${PACKAGE}_${TYPE}_LIBRARIES)

    if(DEFINED ${PACKAGE}_LIBRARY_${TYPE} AND ${PACKAGE}_LIBRARY_${TYPE})
      list(APPEND _libs "${${PACKAGE}_LIBRARY_${TYPE}}")
    endif(DEFINED ${PACKAGE}_LIBRARY_${TYPE} AND ${PACKAGE}_LIBRARY_${TYPE})
    if(DEFINED ${PACKAGE}_LIBRARIES_${TYPE} AND ${PACKAGE}_LIBRARIES_${TYPE})
      foreach(_lib ${${PACKAGE}_LIBRARIES_${TYPE}})
        list(APPEND _libs "${_lib}")
      endforeach(_lib)
    endif(DEFINED ${PACKAGE}_LIBRARIES_${TYPE} AND ${PACKAGE}_LIBRARIES_${TYPE})
  else(NOT TYPE STREQUAL "")
    if(DEFINED ${PACKAGE}_LIBRARY AND ${PACKAGE}_LIBRARY)
      list(APPEND _libs "${${PACKAGE}_LIBRARY}")
    endif(DEFINED ${PACKAGE}_LIBRARY AND ${PACKAGE}_LIBRARY)
    if(DEFINED ${PACKAGE}_LIBRARIES AND ${PACKAGE}_LIBRARIES)
      foreach(_lib ${${PACKAGE}_LIBRARIES})
        list(APPEND _libs "${_lib}")
      endforeach(_lib)
    endif(DEFINED ${PACKAGE}_LIBRARIES AND ${PACKAGE}_LIBRARIES)
  endif(NOT TYPE STREQUAL "")

  set(${VAR} "${_libs}" PARENT_SCOPE)
endfunction(package_libraries)

#==============================================================================
# link_packages(TARGET package1 package2 ...)
#
# Link the listed package libraries into TARGET. This handles various ways
# packages can be specified using find_package().
# The function deals with:
# - Library variable naming with <package>_LIBRARY or <package>_LIBRARIES.
#   Henceforth we may use the two interchangeably.
# - Multiple configuration build or single configuration build.
# - Managing separate debug and release libraries and configurations.
#   - Handles <package>_DEBUG_LIBRARIES, <package>_RELEASE_LIBRARIES
#     and <package>_LIBRARIES
# - Package specified as targets, either important or locally defined.
#
# The list of packages is provided in the additional arguments (${ARGN}).
# For each package the following are linked if defined:
# 1. Separate debug and release libraries if defined. Both must be defined to
#    succeed.
#   a. <package>_DEBUG_LIBRARY or <package>_DEBUG_LIBRARIES as debug
#   b. <package>_RELEASE_LIBRARY or <package>_RELEASE_LIBRARIES as optimized
# 2. <package>_LIBRARY or <package>_LIBRARIES
#
# Errors are reported for each package which cannot be linked.
#
# Debug/trace switches:
# LINK_PACKAGES_DBG - the debug output bit field
#   0, false not defined - No debug output
#   bit 0 (1) - Show which libraries are linked to the target.
#   bit 1 (2) - Show the libraries found for each package, split into the debug, release and default lists.
#
# LINK_PACKAGES_TRACE - the libraries for a particular package.
#   Restricts the LINK_PACKAGES_DBG logging to the package name listed in LINK_PACKAGES_TRACE.
#==============================================================================
function(link_packages TARGET)
  #set(LINK_PACKAGES_DBG 3)
  set(LINK_PACKAGES_DBG_LINKED 0)
  set(LINK_PACKAGES_DBG_LIST 0)
  if (LINK_PACKAGES_DBG)
    math(EXPR LINK_PACKAGES_DBG_LINKED "${LINK_PACKAGES_DBG} & 1")
    math(EXPR LINK_PACKAGES_DBG_LIST "${LINK_PACKAGES_DBG} & 2")
    message("link_packages for ${TARGET}")
    if(LINK_PACKAGES_DBG_LINKED)
      set(LINK_PACKAGES_DBG_LINKED YES)
    endif(LINK_PACKAGES_DBG_LINKED)
    if(LINK_PACKAGES_DBG_LIST)
      set(LINK_PACKAGES_DBG_LIST YES)
    endif(LINK_PACKAGES_DBG_LIST)
    message("List linked libraries? ${LINK_PACKAGES_DBG_LINKED}")
    message("List package libraries? ${LINK_PACKAGES_DBG_LIST}")
    if(LINK_PACKAGES_TRACE)
      message("Only for package: ${LINK_PACKAGES_TRACE}")
    endif(LINK_PACKAGES_TRACE)
  endif(LINK_PACKAGES_DBG)

  foreach(PACKAGE ${ARGN})
    set(_linked NO)

    # Check if it's a target for easy linking.
    if(TARGET ${PACKAGE})
      target_link_libraries(${TARGET} ${PACKAGE})
      set(_linked YES)
    else(TARGET ${PACKAGE})
      # Check for differentiated debug and release libraries.
      package_libraries(_DEBUG_LIBRARIES ${PACKAGE} DEBUG)
      package_libraries(_RELEASE_LIBRARIES ${PACKAGE} RELEASE)
      package_libraries(_DEFAULT_LIBRARIES ${PACKAGE} "")

      if (LINK_PACKAGES_DBG_LIST AND (NOT LINK_PACKAGES_TRACE OR LINK_PACKAGES_TRACE STREQUAL PACKAGE))
        message("${PACKAGE}_DEBUG_LIBRARIES: ${_DEBUG_LIBRARIES}")
        message("${PACKAGE}_RELEASE_LIBRARIES: ${_RELEASE_LIBRARIES}")
        message("${PACKAGE}_DEFAULT_LIBRARIES: ${_DEFAULT_LIBRARIES}")
      endif(LINK_PACKAGES_DBG_LIST AND (NOT LINK_PACKAGES_TRACE OR LINK_PACKAGES_TRACE STREQUAL PACKAGE))

      if(LINK_PACKAGES_DBG_LINKED)
        set(LINK_PACKAGES_DBG_LINKED_PKG YES)
        if(LINK_PACKAGES_TRACE AND NOT LINK_PACKAGES_TRACE STREQUAL PACKAGE)
          set(LINK_PACKAGES_DBG_LINKED_PKG NO)
        endif(LINK_PACKAGES_TRACE AND NOT LINK_PACKAGES_TRACE STREQUAL PACKAGE)
      endif(LINK_PACKAGES_DBG_LINKED)

      if(_DEBUG_LIBRARIES AND (_RELEASE_LIBRARIES OR _DEFAULT_LIBRARIES))
        if(NOT _RELEASE_LIBRARIES)
          set(_RELEASE_LIBRARIES ${_DEFAULT_LIBRARIES})
        endif(NOT _RELEASE_LIBRARIES)

        # Debug libraries present. Use with "debug" prefix.
        # Use either <PACKAGE>_RELEASE_LIBRAR(Y|IES) or <PACKAGE>_LIBRAR(Y|IES)
        # for the "opitmized" libraries.
        set(_linked YES)
        # Link debug.
        foreach(_lib ${_DEBUG_LIBRARIES})
          if(NOT _lib MATCHES "^(debug)|(optimzied)|(general)$")
            if(LINK_PACKAGES_DBG_LINKED_PKG)
              message("target_link_libraries(${TARGET} debug ${_lib})")
            endif(LINK_PACKAGES_DBG_LINKED_PKG)
            target_link_libraries(${TARGET} debug "${_lib}")
          endif(NOT _lib MATCHES "^(debug)|(optimzied)|(general)$")
        endforeach(_lib)

        # Link release
        foreach(_lib ${_RELEASE_LIBRARIES})
          if(NOT _lib MATCHES "^(debug)|(optimzied)|(general)$")
            if(LINK_PACKAGES_DBG_LINKED_PKG)
              message("target_link_libraries(${TARGET} optimized ${_lib})")
            endif(LINK_PACKAGES_DBG_LINKED_PKG)
            target_link_libraries(${TARGET} optimized "${_lib}")
          endif(NOT _lib MATCHES "^(debug)|(optimzied)|(general)$")
        endforeach(_lib)
      endif(_DEBUG_LIBRARIES AND (_RELEASE_LIBRARIES OR _DEFAULT_LIBRARIES))

      if(NOT _linked)
        if(NOT _DEFAULT_LIBRARIES AND _RELEASE_LIBRARIES)
          # Only Release libraries. Use that list.
          set(_DEFAULT_LIBRARIES ${_RELEASE_LIBRARIES})
        endif(NOT _DEFAULT_LIBRARIES AND _RELEASE_LIBRARIES)

        if(_DEFAULT_LIBRARIES)
          set(_linked YES)
            if(LINK_PACKAGES_DBG_LINKED_PKG)
              message("target_link_libraries(${TARGET} ${_DEFAULT_LIBRARIES})")
            endif(LINK_PACKAGES_DBG_LINKED_PKG)
          target_link_libraries(${TARGET} ${_DEFAULT_LIBRARIES})
        endif(_DEFAULT_LIBRARIES)
      endif(NOT _linked)

      if(NOT _linked)
        message(SEND_ERROR "Unable to find ${TARGET} link libraries for ${PACKAGE}")
      endif(NOT _linked)

      set(_DEBUG_LIBRARIES)
      set(_RELEASE_LIBRARIES)
      set(_DEFAULT_LIBRARIES)
    endif(TARGET ${PACKAGE})
  endforeach(PACKAGE)
endfunction(link_packages)


#===============================================================================
# Configures a config header file for a project. The config file must lie in the
# of the project (same directory as the CMakeLists.txt file) and be of the form:
# <config_name>.h.in. The function is then invoked:
#   target_config_file(TARGET <config_name>.h [SHARED] [VMAJOR 1] [VMINOR 0] [VAR var])
# Parameters:
#   CONFIG_FILE The name of the file to configure. This is a header file name.
#     The actual file configured has a ".in" suffix, which is removed on
#     configuring. The argument is specified without the suffix.
#   VMAJOR n (Optional) Major version number. Written to @VERSION_MAJOR@ in the
#     config file.
#   VMINOR n (Optional) Minor version number. Written to @VERSION_MINOR@ in the
#     config file.
#   SHARED (Optional) Specifies a shared library is being built. This sets a
#     variable ${TARGET_IDENTIFIER}_SHARED before configuring the file.
#   VAR Specifies a variable name into which to write the full path of the configured
#     file.
#
# Config file may contain the following values to configure:
#   @TARGET_HEADER_GUARD@ as part of the header guard. Replaced with the target
#     name, replacing invalid characters with "_".
#   @TARGET_IDENTIFIER@ The target name, in upper case with non-identifier characters
#     replaced with "_"
#   @TARGET_NAME@ The name of the target being built.
#   @VERSION_MAJOR@ Replaced with the value of VMAJOR.
#   @VERSION_MINOR@ Replaced with the value of VMINOR.
#
# The configured file is written to the ${CMAKE_CURRENT_BINARY_DIR}.
#===============================================================================
function(target_config_file TARGET CONFIG_FILE)
  CMAKE_PARSE_ARGUMENTS(CONFIG_FILE "SHARED" "VAR;VMAJOR;VMINOR" "" ${ARGN})

  # Setup version numbers.
  if (DEFINED CONFIG_FILE_VMAJOR)
    set(VERSION_MAJOR ${CONFIG_FILE_VMAJOR})
  else(DEFINED CONFIG_FILE_VMAJOR)
    set(VERSION_MAJOR 1)
  endif(DEFINED CONFIG_FILE_VMAJOR)
  if (DEFINED CONFIG_FILE_VMINOR)
    set(VERSION_MINOR ${CONFIG_FILE_VMINOR})
  else(DEFINED CONFIG_FILE_VMINOR)
    set(VERSION_MINOR 0)
  endif(DEFINED CONFIG_FILE_VMINOR)
  
  string(REGEX  REPLACE "[ -]" "_" TARGET_IDENTIFIER ${TARGET})
  set(${TARGET_IDENTIFIER}_SHARED 0)
  if (CONFIG_FILE_SHARED)
    set(${TARGET_IDENTIFIER}_SHARED 1)
  endif(CONFIG_FILE_SHARED)
  
  # Set copyright year.
  set(YEAR x)
  
  #message(STATUS "Configuring ${CONFIG_FILE}.in")
  # Overwrite PROJECT name to remove illegal characters.
  string(TOUPPER ${TARGET_IDENTIFIER} TARGET_HEADER_GUARD)
  configure_file("${CONFIG_FILE}.in" "${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE}")
  if(CONFIG_FILE_VAR)
    set(${CONFIG_FILE_VAR} "${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE}" PARENT_SCOPE)
  endif(CONFIG_FILE_VAR)
endfunction(target_config_file)

#===============================================================================
# Sets the desired output subdirectory for the current target.
# Must be called between target_begin() and target_end(), but does not take
# effect on the target properties until after the target is defined in
# target_end()
#
# SUBDIR - the subdirectory to append.
#===============================================================================
macro(target_output_subdir SUBDIR)
  set(TARGET_SUBDIR "${SUBDIR}")
endmacro(target_output_subdir)

#===============================================================================
# A helper function for target_output_subdir().
# Appends a subdirectory to one of the target output directory properties.
#
# Must be called after target_end().
#
# BASE_DIR_TYPE - the output directory type. Prepended to _OUTPUT_DIRECTORY to
#   generate the output property name.
#   Possible values:
#     ARCHIVE
#     LIBRARY
#     PDB
#     RUNTIME
# TARGET - the name of the target to modify output for.
# SUBDIR - the subdirectory to append.
#===============================================================================
function(append_target_output_dir BASE_DIR_TYPE TARGET SUBDIR)
  get_target_property(BASE_OUT_DIR ${TARGET} "${BASE_DIR_TYPE}_OUTPUT_DIRECTORY")
  if(CMAKE_CONFIGURATION_TYPES)
    foreach(CONF ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER ${CONF} CONFU)
      get_target_property(CURDIR ${TARGET} "${BASE_DIR_TYPE}_OUTPUT_DIRECTORY_${CONFU}")
      if(NOT CURDIR)
        set(CURDIR "${CMAKE_${BASE_DIR_TYPE}_OUTPUT_DIRECTORY}")
        if(NOT CURDIR)
          set(CURDIR "${BASE_OUT_DIR}")
        endif(NOT CURDIR)
      endif(NOT CURDIR)
      #message("set_target_properties(${TARGET} PROPERTIES ${BASE_DIR_TYPE}_OUTPUT_DIRECTORY_${CONFU} ""${CURDIR}/${SUBDIR}"")")
      set_target_properties(${TARGET} PROPERTIES ${BASE_DIR_TYPE}_OUTPUT_DIRECTORY_${CONFU} "${CURDIR}/${SUBDIR}")
    endforeach(CONF)
  else(CMAKE_CONFIGURATION_TYPES)
    set_target_properties(${TARGET} PROPERTIES ${BASE_DIR_TYPE}_OUTPUT_DIRECTORY "${BASE_OUT_DIR}/${SUBDIR}")
  endif(CMAKE_CONFIGURATION_TYPES)
endfunction(append_target_output_dir BASE_DIR_TYPE TARGET SUBDIR)

#===============================================================================
# Modifies a target's output directory properties by appending SUBDIR.
# Must be called after target_end().
#
# TARGET - the name of the target to modify output for.
# SUBDIR - the subdirectory to append.
#===============================================================================
function(target_set_output_subdir TARGET SUBDIR)
  append_target_output_dir(ARCHIVE ${TARGET} "${SUBDIR}")
  append_target_output_dir(LIBRARY ${TARGET} "${SUBDIR}")
  append_target_output_dir(RUNTIME ${TARGET} "${SUBDIR}")
endfunction(target_set_output_subdir)


#===============================================================================
# Helper function to automatically add a suffix to the build files. The suffix
# matches the build configuration by expanding ${TARGET_SUFFIX_${CONFIG}},
# with defaults as follows:
# - Debug : d
# - MinSizeRel : m
# - Release : <none>
# - RelWithDebInfo : i
#
# Usage:
#   target_auto_suffix(<target>)
#===============================================================================
function(target_auto_suffix TARGET)
  if(CMAKE_BUILD_TYPE)
    # Individual build configuration specified.
    set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${TARGET}${TARGET_SUFFIX_${CMAKE_BUILD_TYPE}})
  else(CMAKE_BUILD_TYPE)
    foreach(CONF ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER ${CONF} CONFU)
      set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME_${CONFU} ${TARGET}${TARGET_SUFFIX_${CONF}})
    endforeach(CONF)
  endif(CMAKE_BUILD_TYPE)
endfunction(target_auto_suffix)

#===============================================================================
# Helper function which applies the use of a precompiled header file. This is
# invoked for a target after defining that target.
#
# Usage:
#   target_implement_pch(<target-name> <header-file> [FORCE] [VM <size>])
#
# <target-name> : The name of the target.
# <header-file> : The precompiled header include file.
# FORCE : When present, forces the <header-file> to be included in all sources
#   in the target.
# VM <size> : VM size for Visual Studio (/Zm<size> option).
#===============================================================================
function(target_implement_pch TARGET HEADER)
  CMAKE_PARSE_ARGUMENTS(TIP "FORCE" "VM" "" "${ARGN}")
  get_filename_component(pchFileName ${HEADER} NAME_WE)

  if(MSVC)
    set(PCH_GENERATE_FLAGS "/Yc${HEADER}")
    set(PCH_USE_FLAGS "/Yu${HEADER}")

    if(TIP_VM)
      set(PCH_GENERATE_FLAGS "${PCH_GENERATE_FLAGS} /Zm${TIP_VM}")
      set(PCH_USE_FLAGS "${PCH_USE_FLAGS} /Zm${TIP_VM}")
    endif(TIP_VM)

    if(TIP_FORCE)
      set(PCH_USE_FLAGS "${PCH_USE_FLAGS} /FI${HEADER}")
    endif(TIP_FORCE)

    get_target_property(sources ${TARGET} SOURCES)
    foreach(_source ${sources})
      set(PCH_COMPILE_FLAGS "")
      if(_source MATCHES \\.\(cc|cxx|cpp\)$)
        get_filename_component(_sourceWe ${_source} NAME_WE)
        # Test for PCH source file.
        if(_sourceWe STREQUAL ${pchFileName})
          # Found PCH source file. Set flags to compile to generate PCH file
          set(PCH_COMPILE_FLAGS "${PCH_GENERATE_FLAGS}")
          set(_sourceFound TRUE)
        else(_sourceWe STREQUAL ${pchFileName})
          # General source. Mark file to use PCH file.
          set(PCH_COMPILE_FLAGS "${PCH_USE_FLAGS}")
        endif(_sourceWe STREQUAL ${pchFileName})
      set_source_files_properties(${_source} PROPERTIES COMPILE_FLAGS "${PCH_COMPILE_FLAGS}")
      endif(_source MATCHES \\.\(cc|cxx|cpp\)$)
    endforeach(_source)
    if(NOT _sourceFound)
      message(FATAL_ERROR "Matching Precompiled Header source file not found for header: ${HEADER}.")
    endif(NOT _sourceFound)
  endif(MSVC)
 
  if(CMAKE_COMPILER_IS_GNUCXX)
    get_filename_component(_name ${HEADER} NAME)
    set(_source "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}")
    set(_outdir "${CMAKE_CURRENT_BINARY_DIR}/${_name}.pch")
    MAKE_DIRECTORY(${_outdir})
    set(_output "${_outdir}/.c++")
    
    string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
    set(_compiler_FLAGS ${${_flags_var_name}})
    
    get_directory_property(_directory_flags INCLUDE_DIRECTORIES)
    foreach(item ${_directory_flags})
      list(APPEND _compiler_FLAGS "-I${item}")
    endforeach(item)
 
    get_directory_property(_directory_flags DEFINITIONS)
    list(APPEND _compiler_FLAGS ${_directory_flags})
 
    separate_arguments(_compiler_FLAGS)
    # message("${CMAKE_CXX_COMPILER} -DPCHCOMPILE ${_compiler_FLAGS} -x c++-header -o {_output} ${_source}")
    add_custom_command(
      OUTPUT ${_output}
      COMMAND ${CMAKE_CXX_COMPILER} ${_compiler_FLAGS} -x c++-header -o ${_output} ${_source}
      DEPENDS ${_source} )
    add_custom_target(${TARGET}_gch DEPENDS ${_output})
    add_dependencies(${TARGET} ${TARGET}_gch)
    set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "-include ${_name} -Winvalid-pch")
  # elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #   message("xxxxxxxxxxxxxxxxxxx")
  #   get_filename_component(_name ${HEADER} NAME)

  #   set(_source "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}")
  #   set(_output "${CMAKE_CURRENT_BINARY_DIR}/${_name}.pch")

  #   # Clang precompiled header generation requires a hpp file rather than an h file
  #   # to ensure the header is treated as a C++ header, not a C header.
  #   get_filename_component(_extension ${HEADER} EXT)
  #   if(_extension MATCHES ".[Hh]")
  #     message("mutating to C++ header")
  #     get_filename_component(_nameOnly ${HEADER} NAME_WE)
  #     # C header. Need to copy and generate a C++ header.
  #     set(_source2 ${CMAKE_CURRENT_BINARY_DIR}/${_nameOnly}.hpp)
  #     add_custom_command(
  #       OUTPUT ${_source2}
  #       COMMAND ${CMAKE_COMMAND} -E copy ${_source} ${_source2}
  #       DEPENDS ${_source})
  #     set(_source ${_source2})
  #     message("Using ${_source}")
  #     endif()
    
  #   if(NOT CMAKE_BUILD_TYPE)
  #     set(_flags_var_name CMAKE_BUILD_TYPE_RELEASE)
  #   else(CMAKE_BUILD_TYPE)
  #    string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
  #   endif(NOT CMAKE_BUILD_TYPE)
  #   message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
  #   message("CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
  #   message("${_flags_var_name}: ${${_flags_var_name}}")
  #   message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
  #   set(_compiler_FLAGS ${${_flags_var_name}})
  #   set(_compiler_FLAGS "${CMAKE_CXX_FLAGS} ${${_flags_var_name}}")
  #   message("${_compiler_FLAGS}")
    
  #   get_directory_property(_directory_flags INCLUDE_DIRECTORIES)
  #   foreach(item ${_directory_flags})
  #     set(_compiler_FLAGS "${_compiler_FLAGS} -I${item}")
  #     message(STATUS "set(_compiler_FLAGS \"${_compiler_FLAGS} -I${item}\")")
  #   endforeach(item)
 
  #   get_directory_property(_directory_flags DEFINITIONS)
  #   foreach(item ${_directory_flags})
  #     set(_compiler_FLAGS "${_compiler_FLAGS} -I${item}")
  #     message(STATUS "set(_compiler_FLAGS \"${_compiler_FLAGS} -I${item}\")")
  #   endforeach(item)
 
  #   # message("${CMAKE_CXX_COMPILER} -DPCHCOMPILE ${_compiler_FLAGS} -x c++-header -o {_output} ${_source}")
  #   message(STATUS "
  #   add_custom_command(
  #     OUTPUT ${_output}
  #     COMMAND ${CMAKE_CXX_COMPILER} -cc1 -emit-pch ${_compiler_FLAGS} ${_source} -o ${_output}
  #     DEPENDS ${_source} )
  #   ")
  #   separate_arguments(_compiler_FLAGS)
  #   add_custom_command(
  #     OUTPUT ${_output}
  #     COMMAND ${CMAKE_CXX_COMPILER} -cc1 -emit-pch ${_compiler_FLAGS} ${_source} -o ${_output}
  #     DEPENDS ${_source} )
  #   add_custom_target(${TARGET}_pch DEPENDS ${_output})
  #   add_dependencies(${TARGET} ${TARGET}_pch)
  #   get_target_property(_flags ${TARGET} COMPILE_FLAGS)
  #   if(COMPILE_FLAGS)
  #     list(APPEND _flags "-include-pch ${_output}")
  #   else(COMPILE_FLAGS)
  #     set(_flags "-include-pch ${_output}")
  #   endif(COMPILE_FLAGS)
  #   message("set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS ${_flags})")
  #   set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS ${_flags})
  endif(CMAKE_COMPILER_IS_GNUCXX)
endfunction(target_implement_pch)

#==============================================================================
# split_list(VARBASE [MULTIITEM] [SPLIT key1 key2 ...] ITEMS ...)
#
# Splits a list up according to keywords appearing in the list. Splits are
# made where-ever a keyword appears either for the next item only, or until
# a new keyword is found (see MULTIITEM).
#
# The results are inserted into variables in the parent scope named as follows:
# - ${VARBASE}_${key}
# - ${VARBASE}_UNPARSED_ITEMS
# Where each split keyword has a corresponding variable ${VARBASE}_${key} and
# all unspecified items are inserted into ${VARBASE}_UNPARSED_ITEMS.
#
# If MULTIITEM is specified, then all items following a key, until the next
# key are grouped with the preceding key. Otherwise, each key affects only
# the next item (default behaviour).
#
# For example, consider parsing the following list stored in FOOD_LIST:
#   fruit apple pear fruit banana vegetable lettuce vegetable carrot sprouts
# With the split_list call as follows:
#   split_list(FOOD SPLIT fruit vegetable ITEMS ${FOOD_LIST})
# The results are:
# - FOOD_fruit: [apple banana]
# - FOOD_vegetable: [lettuce carrot]
# - FOOD_UNPARSED_ITEMS: [pear sprouts]
# Note how [pear sprouts] are in FOOD_UNPARSED_ITEMS because the default
# behaviour is to match only one item after a key.
#
# With the call changed to:
#   split_list(FOOD MULTIITEM SPLIT fruit vegetable ITEMS ${FOOD_LIST})
# The results are:
# - FOOD_fruit: [apple pear banana]
# - FOOD_vegetable: [lettuce carrot sprouts]
# - FOOD_UNPARSED_ITEMS: []
#==============================================================================
function(split_list VARBASE)
  CMAKE_PARSE_ARGUMENTS(SL "MULTIITEM" "" "SPLIT;ITEMS" "${ARGN}")
  if(NOT SL_MULTIITEM)
    CMAKE_PARSE_ARGUMENTS(SLS "" "${SL_SPLIT}" "" "${SL_ITEMS}")
  else(NOT SL_MULTIITEM)
    CMAKE_PARSE_ARGUMENTS(SLS "" "" "${SL_SPLIT}" "${SL_ITEMS}")
  endif(NOT SL_MULTIITEM)
  foreach(SPLIT ${SL_SPLIT})
    set(${VAR_BASE}_${SPLIT} "${SLS_${SPLIT}}" PARENT_SCOPE)
  endforeach(SPLIT)
  set(${VAR_BASE}_UNPARSED_ITEMS "${SLS_UNPARSED_ARGUMENTS}" PARENT_SCOPE)
endfunction(split_list)
