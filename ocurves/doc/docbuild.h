//
// @author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef DOCBUILD_H_
#define DOCBUILD_H_

/**
@page build Building Open Curves

@tableofcontents

Open Curves has been built as a multi-platform application by leveraging the Qt framework and CMake.
As such, building Open Curves is generally straightforward, but there may be some issues for some build
configurations. The primary platform for Open Curves is Windows 7, but it is also commonly built for
MacOS.

Dependencies
============
Building Open Curves requires the following tools:
- A compatible compiler
- <a href="http://www.qt.io/download-open-source/">Qt 5.2</a> or above with the following modules:
  - Core
  - Concurrent
  - GUI
  - Network
  - OpenGL (for Widgets)
  - PrintSupport
  - SerialPort
  - SVG
  - Widgets
  - XML
- <a href="http://qwt.sourceforge.net/">Qwt</a> in either source or binary form.
- <a href="https://cmake.org/download/">CMake</a> 2.8.12 or above.

Optional build components:
- Bison and Flex to change expression parsing.
- <a href="http://www.stack.nl/~dimitri/doxygen/">Doxygen</a> for generating API documentation.

Open Curves supports building against existing Qwt shared libraries, or building Qwt from source.
To use an existing Qwt library, that library must be built against the same version of Qt and
must be locatable by CMake. A full discussion of how CMake find libraries is beyond the scope of
this document.
To build Qwt from source:
-# Download and unpack the Qwt source.
-# Copy the Qwt "src" directory to <tt>\<open-curves-source-dir\>/qwt/src</tt>
Where <tt>\<open-curves-source-dir\></tt> is the Open Curves source directory containing the topmost
CMakeLists.txt file.

Bison and Flex are only required if making changes to the expression grammar or parser. See below.

Compiler Requirements
=====================
Open Curves requires a C++11 compatible compiler. The following compilers are recommended:
Platform  | Recommended Compiler
--------- | -------------------------------------------
Windows   | Visual Studio 2013 or above.
MacOS     | Xcode 6 or higher. Apple Clang 7 or higher.
Linux     | G++ 4.8 or higher, 4.9 or higher preferred.

Linux builds have only received cursory testing to date.

Build Process
=============
CMake is required to set up the build projects or make files. CMake is run in a command line shell
such as an XTerm window or Windows CMD.

Before running it is advised that:
- The CMake executable is available on the path.
- The Qt bin directory is available on the path. This is the path to the qmake executable.
- Qwt libraries are locatable by CMake or the source code is available as described above.

Open the command shell and proceed with the following instructions.
-# Navigate to the Open Curves source directory.
-# Create a "build" directory within the Open Curves source directory.
-# Change into the build directory.
-# Build and install
  - On Linux or MacOS:
    - run "make"
    - run "make ocurves-html" to build API documentation
    - run "make userdoc" if Doxygen is installed
    - run "make install"
  - On Windows:
    - Open the Visual Studio project and build.
    - Build the "ocurves-html" project to generate API documentation.
    - Build the "ocurves-marshall" project after the first time each configuration is built.
    - Build the userdoc target before attempting installation.
    - To install, build the INSTALL target. Administrator privileges are required to install to
      "Program Files" directories.

The installation directory can be changed by modifying the CMAKE_INSTALL_PREFIX. This can be done
in a number of ways:
-# When first running cmake, change the command to read:
    <tt>cmake -DCMAKE_INSTALL_PREFIX=<install-dir> ..</tt>
  where \<install-dir\> is the desired installation directory.
-# Use cmake-gui or ccmake to change CMAKE_INSTALL_PREFIX. From the build directory run.
    <tt>ccmake ..</tt> or <tt>cmake-gui ..</tt>


Platform Notes
--------------
### Windows ###
On Windows, the CMake scripts are set to marshal dependent DLLs to the build directory. The
project "ocurves-marshal" is responsible for copying dependencies - such as Qt DLLs - to the
build directory. This project does not build by default because it can take some time to complete.
It must be build manually for each build configuration. This need only be done when the
dependencies change, such as when Qt is upgraded. The install script marshals dependencies
independently and does not reply on the "ocurves-marshal" project.

#### Windows 64-bit ####
64-bit Windows code can be built by modifying the CMake command line to read:

    cmake -G "Visual Studio 14 2015 Win64"

or

    cmake -G "Visual Studio 12 2013 Win64"

Note the addition of "Win64". This requires 64-bit Qt and Qwt binaries.

### Linux ###
The relative installation paths have not been verified on Linux systems. That is to say, the
installation pattern will be non-standard. It is advised that until this is resolved, a
non-standard installation path be used, one which isolates Open Curves from system applications.

Advanced Options
----------------
### Alloctrack ###
The source includes an library called "alloctrack", which is designed to aid in memory leak
detection. It is disabled by default. Alloctrack is enabled by setting the CMake variable
ALLOCTRACK_ENABLE to TRUE using either the command line, ccmake or cmake-gui. Alloctrack is
strongly recommended for debug builds, but is not used release builds and should be disabled.

### Bison and Flex ###
Expression parsing uses Bison and Flex. The generated files are included in the submitted source,
so they are optional components. However, they are required when modifying the expression parsing.

*/

#endif // DOCBUILD_H_
