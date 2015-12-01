cmake_minimum_required(VERSION 2.8.12)
if(CMAKE_VERSION VERSION_GREATER 2.8.13)
  cmake_policy(SET CMP0028 OLD) # Allow Qt use of "::" in target names.
  cmake_policy(SET CMP0043 OLD) # Cater for Qt target compile definitions
endif(CMAKE_VERSION VERSION_GREATER 2.8.13)

#set(MACOSX_I386 NO)

project(OpenCurves)

# Add the known scripts directory to the modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")

file(TO_CMAKE_PATH "$ENV{CMAKE_MODULE_PATH}" MODULES_PATH)
if(MODULES_PATH)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${MODULES_PATH}")
endif(MODULES_PATH)

# Find and enable C++11 support.
find_package(CPP11)
if(CPP11_FLAGS)
  add_definitions("${CPP11_FLAGS}")
endif(CPP11_FLAGS)

include(core)
include(installinfo.cmake)

# Set common output directories to, per configuration if needed.
target_set_output_directories(MULTI)

# Build options
option(ALLOCTRACK_ENABLE "Build and use Alloctrack leak tracking library?" NO)

# Find Qwt. We build it the source code is present.
# Otherwise we try find a built library.
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/qwt/src/qwt.h")
  add_subdirectory(qwt)
  set(QWT_PACKAGE qwt)
  add_definitions(-DQWT_DLL)
else(EXISTS "${CMAKE_CURRENT_LIST_DIR}/qwt/src/qwt.h")
  find_package(Qwt)
  set(QWT_PACKAGE Qwt)
endif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/qwt/src/qwt.h")
if(ALLOCTRACK_ENABLE)
  add_subdirectory(alloctrack)
endif(ALLOCTRACK_ENABLE)
add_subdirectory(userdoc)
add_subdirectory(plots)
add_subdirectory(ocurves)
add_subdirectory(netgraph)

if(ALLOCTRACK_ENABLE)
  add_doxygen_directories("${CMAKE_CURRENT_LIST_DIR}/alloctrack")
endif(ALLOCTRACK_ENABLE)
set(DOXYGEN_EXAMPLE_PATH "${CMAKE_CURRENT_LIST_DIR}")
generate_doxygen_target(ocurves-html PROJECT "Open Curves" VERSION 0.9)

install(FILES README.md release-notes.txt DESTINATION "${OCURVES_SUPPORT_TARGET_DIR}" COMPONENT Runtime)
include(InstallRequiredSystemLibraries)
install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION "${OCURVES_BINARY_TARGET_DIR}" COMPONENT Runtime)

#-------------------------------------------------------------------------------
# Marshalling DLLs to the build directory (not installation, Windows only)
#-------------------------------------------------------------------------------
find_package(Qt5Core)
get_target_property(QMAKE_LOC Qt5::qmake LOCATION_Release)
get_filename_component(QT_BINARY_DIR "${QMAKE_LOC}" DIRECTORY)
marshal_exe_dependencies(ocurves-marshal ${OCURVES_APP_NAME} PATHS "${QT_BINARY_DIR}")

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER core)
