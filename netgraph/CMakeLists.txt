set(TARGET netgraph)

if(WIN32)
  set(QT_USE_QTMAIN TRUE)
endif(WIN32)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# OpenGL, PrintSupport and SVG are not directly required for compiling, but required to run.
find_package(Qt5 CONFIG REQUIRED
  Core
  Network
)

# This need only be enabled if another version of Qt may exist on the system path.
# For example, on OSX, Qt4 may be installed on the framework paths and included first
# via the use of some other framework library. The following line ensures Qt5 as
# found by find_qt5() is included first.

# List source files. Add additional directories here. Defaults to find C++ source files.
# Added to "SOURCES" variable.
set(SOURCES
  netgraph.cpp
)

add_executable(netgraph ${SOURCES} ${UI_FILES})
target_include_directories(netgraph PRIVATE "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(netgraph PRIVATE Qt5::Core Qt5::Network)

# CMake does not automatically propagate CMAKE_DEBUG_POSTFIX to executables. We do so to avoid confusing link issues
# which can would when building release and debug exectuables to the same path.
set_target_properties(netgraph PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

# Setup solution display for the target.
# Set the solution folder for the target (optional).
set_target_properties(netgraph PROPERTIES FOLDER utils)

# Setup target versioning.
set_target_properties(netgraph PROPERTIES
  VERSION ${OpenCurves_VERSION}
  SOVERSION 1
)

# Setup installation.
# Binary installation.
install(TARGETS netgraph
  DESTINATION bin COMPONENT Runtime
)

install(FILES tcp.xml DESTINATION bin COMPONENT Runtime)
install(FILES tcpbin.xml DESTINATION bin COMPONENT Runtime)
install(FILES udp.xml DESTINATION bin COMPONENT Runtime)
install(FILES udpbin.xml DESTINATION bin COMPONENT Runtime)

# Install PDB files (MSVC)
if(MSVC)
  install(FILES $<TARGET_PDB_FILE:netgraph> DESTINATION bin OPTIONAL)
endif(MSVC)

# Setup folder display with the target for Visual Studio.
# source_group("generated" FILES ${GENERATED_FILES})
source_group("source" REGULAR_EXPRESSION ".*$")
