#==============================================================================
# CMake macros to help setup doxygen for a project or set of projects.
#
# Author: Kazys Stepanas July 2012
# Copyright CSIRO 2012
#
# Usage:
#   For a single target:
#     add_doxygen_directories(${sources} PARENT_SCOPE)
#     generate_doxygen_target(target-name)
#
#   For collating multiple projects:
#     In the root CMakeLists.txt file:
#       add_subdirectory(a)
#       add_subdirectory(b)
#       add_subdirectory(c)
#       generate_doxygen_target(target-name)
#
#     In each project CMakeLists.txt file:
#       add_doxygen_directories(projectName ${sources})
#
# In some cases, the doxygen generation may occur several levels above the current scope.
# In this case, each CMakeLists file in the chain must call:
#     propagate_doxygen_directories()
# This ensures the doxygen directories are propagated to the parent.
#
# A single project can forcibly generate its own, uncollated doxygen by:
#   clear_doxygen_dirs()
#   add_doxygen_directories(${sources})
#   generate_doxygen_target(target-name)
#   clear_doxygen_dirs()
# This will leave collated directories unchanged for the parent CMakeLists.txt
# file.
#
# Any CMakeLists.txt file that is to generate a doxygen target must include
# a doxyfile.in file for use with the CMake configure_file() command. This file can
# use the following variables:
#   DOXYGEN_PROJECT - The overall project name to document.
#   DOXYGEN_PROJECT_VERSION - The version number of the project.
#   DOXYGEN_INPUT_LIST - The list of input directories. This lists is
#     formatted (note the backslashes and line breaks):
#       \
#       directory1 \
#       directory2 \
#       directory3
#   DOXYGEN_IMAGE_PATH - List of input directories from which to grab images.
# Other variables may be set before invoking doxygen() in non-collate mode.
#
# Typically, the file would contain the following lines:
#
# INPUT = @DOXYGEN_INPUT_LIST@
# SOURCE = @DOXYGEN_IMAGE_PATH@
#
# The file name can be overridden by setting the value of DOXYGEN_TEMPLATE.
#==============================================================================

include(CMakeParseArguments)

if(NOT DOXYGEN_TEMPLATE)
  set(DOXYGEN_TEMPLATE "doxyfile.in")
endif(NOT DOXYGEN_TEMPLATE)

#------------------------------------------------------------------------------
# clear_doxygen_dirs()
#
# Clears doxygen variables.
#------------------------------------------------------------------------------
macro(clear_doxygen_dirs)
  unset(DOXYGEN_DIRS)
  unset(DOXYGEN_INPUT)
  unset(DOXYGEN_IMAGE_DIRS)
  unset(DOXYGEN_IMAGE_INPUT)
  unset(DOXYGEN_EXAMPLE_DIRS)
  unset(DOXYGEN_EXAMPLE_INPUT)
  unset(DOXYGEN_PROJECT)
  unset(DOXYGEN_PROJECT_VERSION)
endmacro(clear_doxygen_dirs)


#------------------------------------------------------------------------------
# add_doxygen_directories(<directories> [PARENT_SCOPE] [PATH item1 item2 ...] [IMAGE_PATH item1 ...])
#
# Adds directories to the list of doxygen directories. Source and header files
# in these directories are included in the documentation.
#
# PARENT_SCOPE: sets the DOXYGEN_DIRS in the PARENT_SCOPE, not the local scope.
#------------------------------------------------------------------------------
macro(add_doxygen_directories)
  CMAKE_PARSE_ARGUMENTS(ADD "PARENT_SCOPE" "" "PATH;IMAGE_PATH;EXAMPLE_PATH" "${ARGN}")
  set(DOXYGEN_INPUT "${DOXYGEN_DIRS}")
  if(ADD_PATH)
    set(ADD_PATH "${ADD_PATH};${ADD_UNPARSED_ARGUMENTS}")
    set(DBG 1)
  else(ADD_PATH)
    set(ADD_PATH "${ADD_UNPARSED_ARGUMENTS}")
  endif(ADD_PATH)

  foreach(dir ${ADD_PATH})
    if(NOT IS_DIRECTORY "${dir}")
      get_filename_component(dir "${dir}" ABSOLUTE)
      get_filename_component(dir "${dir}" DIRECTORY)
    endif(NOT IS_DIRECTORY "${dir}")
    list(APPEND DOXYGEN_INPUT "${dir}")
  endforeach(dir)
  list(REMOVE_DUPLICATES DOXYGEN_INPUT)

  set(DOXYGEN_IMAGE_INPUT "${DOXYGEN_IMAGE_DIRS}")
  foreach(dir ${ADD_IMAGE_PATH})
    if(NOT IS_DIRECTORY "${dir}")
      get_filename_component(dir "${dir}" ABSOLUTE)
      get_filename_component(dir "${dir}" DIRECTORY)
    endif(NOT IS_DIRECTORY "${dir}")
    list(APPEND DOXYGEN_IMAGE_INPUT "${dir}")
  endforeach(dir)
  list(REMOVE_DUPLICATES DOXYGEN_IMAGE_INPUT)

  set(DOXYGEN_EXAMPLE_INPUT "${DOXYGEN_EXAMPLE_DIRS}")
  foreach(dir ${ADD_EXAMPLE_PATH})
    if(NOT IS_DIRECTORY "${dir}")
      get_filename_component(dir "${dir}" ABSOLUTE)
      get_filename_component(dir "${dir}" DIRECTORY)
    endif(NOT IS_DIRECTORY "${dir}")
    list(APPEND DOXYGEN_EXAMPLE_INPUT "${dir}")
  endforeach(dir)
  list(REMOVE_DUPLICATES DOXYGEN_EXAMPLE_INPUT)

  if(ADD_PARENT_SCOPE)
    set(DOXYGEN_DIRS "${DOXYGEN_INPUT}" PARENT_SCOPE)
    set(DOXYGEN_IMAGE_DIRS "${DOXYGEN_IMAGE_INPUT}" PARENT_SCOPE)
    set(DOXYGEN_EXAMPLE_DIRS "${DOXYGEN_EXAMPLE_INPUT}" PARENT_SCOPE)
  endif(ADD_PARENT_SCOPE)

  set(DOXYGEN_DIRS "${DOXYGEN_INPUT}")
  set(DOXYGEN_IMAGE_DIRS "${DOXYGEN_IMAGE_INPUT}")
  set(DOXYGEN_EXAMPLE_DIRS "${DOXYGEN_EXAMPLE_INPUT}")
endmacro(add_doxygen_directories)


#------------------------------------------------------------------------------
# propagate_doxygen_directories()
#
# Propagate DOXYGEN_DIRS to the parent scope.
#------------------------------------------------------------------------------
macro(propagate_doxygen_directories)
  set(DOXYGEN_DIRS "${DOXYGEN_DIRS}" PARENT_SCOPE)
endmacro(propagate_doxygen_directories)

#------------------------------------------------------------------------------
# generate_doxygen_target(DOXYGEN_TARGET
#                         [PROJECT projectName]
#                         [VERSION version]
#                         [CONFIGURATION Debug;MinSizeRel;Release;RelWithDebInfo]
#                         [INSTALL] [COMPONENT component])
#
# Generates a doxygen target called "DOXYGEN_TARGET" which includes the
# current list of doxygen directories. This requires the current CMakeLists
# directory to contain a doxyfile.in file as described at the top of this file.
# Only installs as part of the Release Build configuration.
#
# PROJECT projectName
#   Defines the name of the project as displayed within the doxygen documentation.
#   This sets the DOXYGEN_PROJECT variable. Otherwise the DOXYGEN_TARGET is used.
# CONFIGURATIONS Debug;MinSizeRel;Release;RelWithDebInfo
#   Specifies the build configuration(s) to build the documentation for.
#   Documentation is not built as part of other build configurations.
#   The default is restricted to Release.
# INSTALL
#   Indicates that the documentation should be include in the installation
#   Uses the install() command.
# COMPONENT component
#   Specifies the component name for the install() command. INSTALL must also
#   present.
#------------------------------------------------------------------------------
function(generate_doxygen_target DOXYGEN_TARGET)
  CMAKE_PARSE_ARGUMENTS(GDT "INSTALL" "COMPONENT;PROJECT;VERSION;CSS;OUTDIR" "CONFIGURATIONS" "${ARGN}")

  # Only generate for the top level CMAKE_LISTS file
  find_package(Doxygen QUIET)
  
  if(NOT DOXYGEN_FOUND)
    clear_doxygen_dirs(PARENT_SCOPE)
    return()
  endif(NOT DOXYGEN_FOUND)

  if(NOT DEFINED DOXYGEN_DIRS OR NOT DOXYGEN_DIRS)
    message(SEND_ERROR "Nothing for doxygen to process for project ${DOXYGEN_TARGET}.")
    return()
  endif(NOT DEFINED DOXYGEN_DIRS OR NOT DOXYGEN_DIRS)

  if(DOXYGEN_FOUND)
    if(GDT_OUTDIR)
      set(DOXYGEN_HTML_OUTPUT "${GDT_OUTDIR}")
    else(GDT_OUTDIR)
      set(DOXYGEN_HTML_OUTPUT html)
    endif(GDT_OUTDIR)
    set(DOXYGEN_PROJECT ${DOXYGEN_TARGET})
    if(GDT_PROJECT)
      set(DOXYGEN_PROJECT ${GDT_PROJECT})
    endif(GDT_PROJECT)
    set(DOXYGEN_PROJECT_VERSION 1)
    if(GDT_VERSION)
      set(DOXYGEN_PROJECT_VERSION ${GDT_VERSION})
    endif(GDT_VERSION)

    # Use CMAKE_CURRENT_PROJECT_DIR for collated documentation.
    # Add DOXYGEN_DIRS and ${src_dirs}
    foreach(dir ${DOXYGEN_DIRS})
      set(DOXYGEN_INPUT_LIST "${DOXYGEN_INPUT_LIST} \\\n  ${dir}")
    endforeach(dir)
    foreach(dir ${DOXYGEN_IMAGE_DIRS})
      set(DOXYGEN_IMAGE_PATH "${DOXYGEN_IMAGE_PATH} \\\n  ${dir}")
    endforeach(dir)
    foreach(dir ${DOXYGEN_EXAMPLE_DIRS})
      set(DOXYGEN_EXAMPLE_PATH "${DOXYGEN_EXAMPLE_PATH} \\\n  ${dir}")
    endforeach(dir)

    if(GDT_CSS)
      set(DOXYGEN_CSS "${GDT_CSS}")
    endif(GDT_CSS)

    configure_file("${DOXYGEN_TEMPLATE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
    get_filename_component(DOXYGEN_TEMPLATE_PATH "${DOXYGEN_TEMPLATE}" ABSOLUTE)
    add_custom_target(${DOXYGEN_TARGET}
                      "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
                      DEPENDS
                        "${DOXYGEN_TEMPLATE_PATH}"
                        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
                      )
    # Restrict build configurations. Default to restricting to Release.
    if(NOT GDT_CONFIGURATIONS)
      set(GDT_CONFIGURATIONS Release)
    endif(NOT GDT_CONFIGURATIONS)

    foreach(CONFIG Debug;MinSizeRel;Release;RelWithDebInfo)
      list(FIND GDT_CONFIGURATIONS ${CONFIG} FIND_RESULT)
      if(FIND_RESULT EQUAL -1)
        # Exclude from this configuration.
        set_target_properties(${DOXYGEN_TARGET} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_${CONFIG} True)
      endif(FIND_RESULT EQUAL -1)
    endforeach(CONFIG)

    if(GDT_INSTALL)
      set(ARG_COMPONENT)
      set(ARG_COMPONENT_NAME)
      if(GDT_COMPONENT)
        set(ARG_COMPONENT COMPONENT)
        set(ARG_COMPONENT_NAME ${GDT_COMPONENT})
      endif(GDT_COMPONENT)
      install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/ DESTINATION html ${ARG_COMPONENT} ${ARG_COMPONENT_NAME} CONFIGURATIONS ${GDT_CONFIGURATIONS})
    endif(GDT_INSTALL)
  endif(DOXYGEN_FOUND)
endfunction(generate_doxygen_target)
