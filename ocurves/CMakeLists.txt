# Copyright (c) 2017
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230
#
# Author: Kazys Stepanas

# Find external depedencies.
# Generally, we should specify either CONFIG to use config style scripts, or MODULE for FindPackage scripts.
find_package(Qt5 CONFIG REQUIRED
  Concurrent
  Gui
  Network
  OpenGL
  PrintSupport
  SerialPort
  Svg
  Widgets
  Xml
)

# Setup automatic creation of Qt generated headers.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configure the project config header.
configure_file(ocurvesconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/ocurvesconfig.h")

set(RESOURCES
  icons/clear.png
  icons/connect.png
  icons/lock.png
  icons/move.png
  icons/multi-tool.png
  icons/ocurves.icns
  icons/ocurves.ico
  icons/ocurves.png
  icons/open2.png
  icons/open.png
  icons/reload2.png
  icons/reload.png
  icons/rubber-band.png
  icons/unlock.png
)

set(SOURCES
  model/bookmarks.cpp
  model/bookmarks.h
  model/curves.cpp
  model/curves.h
  model/expressions.cpp
  model/expressions.h
  rt/realtimecommspec.cpp
  rt/realtimecommspec.h
  rt/realtimeconnection.h
  rt/realtimeserialconnection.cpp
  rt/realtimeserialconnection.h
  rt/realtimesourceloader.cpp
  rt/realtimesourceloader.h
  rt/realtimetcpconnection.cpp
  rt/realtimetcpconnection.h
  rt/realtimeudpconnection.cpp
  rt/realtimeudpconnection.h
  rt/rtbinarymessage.cpp
  rt/rtbinarymessage.h
  rt/rtmessage.cpp
  rt/rtmessage.h
  rt/rtstringmessage.cpp
  rt/rtstringmessage.h
  ui/coloursview.cpp
  ui/coloursview.h
  ui/coloursview.ui
  ui/curveproperties.cpp
  ui/curveproperties.h
  ui/curveproperties.ui
  ui/expressionsview.cpp
  ui/expressionsview.h
  ui/expressionsview.ui
  ui/loadprogress.cpp
  ui/loadprogress.h
  ui/loadprogress.ui
  ui/ocurvesui.cpp
  ui/ocurvesui.h
  ui/ocurvesui.ui
  ui/plotdatacurve.cpp
  ui/plotdatacurve.h
  ui/plotpanner.cpp
  ui/plotpanner.h
  ui/plotview.cpp
  ui/plotview.h
  ui/plotviewtoolbar.cpp
  ui/plotviewtoolbar.h
  ui/plotview.ui
  ui/plotzoomer.cpp
  ui/plotzoomer.h
  ui/splitplotview.cpp
  ui/splitplotview.h
  ui/toolbarwidgets.cpp
  ui/toolbarwidgets.h
  ui/toolbarwidgets.ui
  defaultcolours.cpp
  defaultcolours.h
  main.cpp
  ocurvesconfig.h.in
  ocurves.qrc
  ocurves.rc
  ocurvesutil.cpp
  ocurvesutil.h
  ocurvesver.h
  plotexpressiongenerator.cpp
  plotexpressiongenerator.h
  plotfile.cpp
  plotfile.h
  plotfileloader.cpp
  plotfileloader.h
  plotgenerator.cpp
  plotgenerator.h
  qwtrttiext.h
  realtimeplot.cpp
  realtimeplot.h
  stringitems.h
)

set(DOC_HEADERS
  doc/docbuild.h
  doc/docdownloads.h
  doc/docglossary.h
  doc/docgroups.h
  doc/doclicense.h
  doc/docmain.h
  doc/docstyle.h
)

# Build generated header list.
set(GENERATED_FILES
  "${CMAKE_CURRENT_BINARY_DIR}/ocurvesconfig.h"
)

# Generate the executable.
add_executable(ocurves WIN32 ${SOURCES} ${DOC_HEADERS} ${RESOURCES})
# CMake does not automatically propagate CMAKE_DEBUG_POSTFIX to executables. We do so to avoid confusing link issues
# which can would when building release and debug exectuables to the same path.
set_target_properties(ocurves PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

# Setup solution display for the target.
# Set the solution folder for the target (optional).
set_target_properties(ocurves PROPERTIES FOLDER apps)

# Setup target versioning.
set_target_properties(ocurves PROPERTIES
  VERSION ${OpenCurves_VERSION}
  SOVERSION 1
)

# Add include directories to the target
# For executables, all includes can be private (see set(SOURCES ...) above).
target_include_directories(ocurves
  PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
  )

# Link dependencies.
# Properly defined targets will also have their include directories and those of dependencies added by this command.
target_link_libraries(ocurves
  PUBLIC
    plots
    Qt5::Concurrent
    Qt5::Gui
    Qt5::Network
    Qt5::OpenGL
    Qt5::PrintSupport
    Qt5::SerialPort
    Qt5::Svg
    Qt5::Widgets
    Qt5::Xml
)

# Setup installation.
# Binary installation.
install(TARGETS ocurves
  LIBRARY DESTINATION lib COMPONENT Development
  ARCHIVE DESTINATION lib COMPONENT Development
  RUNTIME DESTINATION bin COMPONENT Runtime
  INCLUDES DESTINATION include COMPONENT Development
)

# Install PDB files (MSVC)
if(MSVC)
  install(FILES $<TARGET_PDB_FILE:ocurves> DESTINATION bin OPTIONAL)
endif(MSVC)

# Setup folder display with the target for Visual Studio.
source_group("generated" FILES ${GENERATED_FILES})
source_group("source" REGULAR_EXPRESSION ".*$")
source_group("source\\doc" REGULAR_EXPRESSION "/doc/.*$")
source_group("source\\icons" REGULAR_EXPRESSION "/icons/.*$")
source_group("source\\model" REGULAR_EXPRESSION "/model/.*$")
source_group("source\\rt" REGULAR_EXPRESSION "/rt/.*$")
source_group("source\\ui" REGULAR_EXPRESSION "/ui/.*$")
