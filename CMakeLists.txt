# Copyright (c) 2017
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230
#
# Author: Kazys Stepanas

# General CMake usage notes:
# - Always quote any variables which may contain directories such as "${CMAKE_LISTS_DIR}".
#   This ensures correct handling for paths containing spaces.

# Project setup and minimum cmake version.
cmake_minimum_required(VERSION 3.5)

# Setup project details.
project(OpenCurves)
set(OpenCurves_VERSION 0.9.1)
set(ConfigPackageLocation lib/cmake/${CMAKE_PROJECT_NAME})

# C++ standards setup.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
# Ensure -fPIC is added.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CLANG_TIDY_PROJECT_PREFIX OpenCurves)
# include(cmake/clang-tidy.cmake)

# If there are additional find scripts in the cmake directory, enable the following to
# add cmake directory to the modules path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(Qwt)

# Ensure debug libraries are built with different named to release builds. This is to address issues such as MSVC
# having different debug and release runtime libraries. For a well setup API, one which hides resource allocation and
# ensures symmetrical deallcation occurs from the same allocator, this won't be a problem, but the consistency is
# useful.
set(CMAKE_DEBUG_POSTFIX "d")
# Marshall all binaries to the same directory. This is expecially useful on Windows when trying to run exectuables from
# this project with shared libraries. Otherwise those shared libraries aren't on the path. Note that other package
# binaries should be on the path already.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Build options
option(OPENCURVES_BUILD_DOXYGEN "Build doxgen documentation? Does not affect user documentation build." OFF)

option(ALLOCTRACK_ENABLE "Build and use Alloctrack leak tracking library?" NO)

# Allow the use of folders to group targets in supporting environments.
# For example Visual Studio solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Manage compiler warnings.
# Use CMAKE_MODULE_PATH and include(warnings) if warnings.cmake is moved.
include(cmake/compilerSetup.cmake)

# Include sub projects.
# Find Qwt. We build it the source code is present.
# Otherwise we try find a built library.
if(ALLOCTRACK_ENABLE)
  add_subdirectory(alloctrack)
endif(ALLOCTRACK_ENABLE)
add_subdirectory(userdoc)
add_subdirectory(plots)
add_subdirectory(ocurves)
add_subdirectory(netgraph)

# Package configuration script configuration.
# Setup import scripts.
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${OpenCurves_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# Installation of the package configuration file.
configure_file(cmake/${CMAKE_PROJECT_NAME}Config.cmake "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake" COPYONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
  COMPONENT Devel)

# Install MSVC runtime libraries. This will also affect the CPack installation.
include(InstallRequiredSystemLibraries)

# Doxygen setup.
if(OPENCURVES_BUILD_DOXYGEN)
  # Include Doxygen helper functions. This also finds the Doxygen package.
  include(cmake/doxygen.cmake)

  if(DOXYGEN_FOUND)
    # Create a target to build the documentation.
    # Here we also setup various documentation variables passed through to the doxyfile configuration.
    # Each named argument below describes the Doxygen variable it sets.
    doxygen_create(
      # DOXYFILE cmake/doxyfile.in  # Doxyfile to configure.
      PROJECT ${CMAKE_PROJECT_NAME} # PROJECT_NAME
      VERSION ${OpenCurves_VERSION}   # PROJECT_NUMBER
      OUTPUT_DIR html               # HTML_OUPTUT
      # CSS <style>.css             # HTML_STYLESHEET
      PUBLISHER "CSIRO"             # DOCSET_PUBLISHER_NAME
      PUBLISHER_ID au.csiro         # DOCSET_PUBLISHER_ID
      PROJECT_ID au.csiro.OpenCurves  # DOCSET_BUNDLE_ID, QHP_NAMESPACE, ECLIPSE_DOC_ID
      PATHS                         # INPUT (RECURSIVE is on)
        ocurves
        plots
        userdoc
      # EXCLUDE_PATHS                 # EXCLUDE
      #   alib/private
      # Where to find source code examples.
      EXAMPLE_PATHS                # EXAMPLE_PATH
        .
      # Where to find images.
      IMAGE_PATHS                   # IMAGE_PATH
        ocurves/icons
        userdoc/images
    )

    # Setup installation of the generated documentation: source, destination.
    doxygen_install(${CMAKE_CURRENT_BINARY_DIR}/html OpenCurves)
  endif(DOXYGEN_FOUND)
endif(OPENCURVES_BUILD_DOXYGEN)
