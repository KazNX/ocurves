set(TARGET netgraph)

if(WIN32)
  set(QT_USE_QTMAIN TRUE)
endif(WIN32)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# OpenGL, PrintSupport and SVG are not directly required for compiling, but required to run.
find_package(Qt5Core)
find_package(Qt5Network)
# This need only be enabled if another version of Qt may exist on the system path.
# For example, on OSX, Qt4 may be installed on the framework paths and included first
# via the use of some other framework library. The following line ensures Qt5 as
# found by find_qt5() is included first.

# Set include directories.
include_directories(. "${CMAKE_CURRENT_BINARY_DIR}")

include_directories(${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Network_INCLUDE_DIRS}
  )

# List source files. Add additional directories here. Defaults to find C++ source files.
# Added to "SOURCES" variable.
list_source_files(SOURCES DIRS .)
list(APPEND SOURCES ${QT_SOURCES})

# Ensure Visual Studio grouping matches that on disk.
group_source_by_dir(${SOURCES} ${UI_FILES})

include_packages(${PACKAGES})
add_executable(${TARGET} ${TARGET_FLAGS} ${SOURCES} ${UI_FILES})
link_packages(${TARGET} ${PACKAGES})

# Link Qt5 modules
qt5_use_modules(${TARGET} Core Network)
target_auto_suffix(${TARGET})

install(TARGETS ${TARGET}
  DESTINATION "${OCURVES_BINARY_TARGET_DIR}" COMPONENT Runtime
  #ARCHIVE DESTINATION lib COMPONENT Development
  #LIBRARY DESTINATION lib COMPONENT Development
  )
install(FILES tcp.xml DESTINATION "${OCURVES_SUPPORT_TARGET_DIR}" COMPONENT Runtime)
install(FILES tcpbin.xml DESTINATION "${OCURVES_SUPPORT_TARGET_DIR}" COMPONENT Runtime)
install(FILES udp.xml DESTINATION "${OCURVES_SUPPORT_TARGET_DIR}" COMPONENT Runtime)
install(FILES udpbin.xml DESTINATION "${OCURVES_SUPPORT_TARGET_DIR}" COMPONENT Runtime)
