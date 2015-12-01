set(TARGET qwt)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)
find_package(Qt5OpenGL)
find_package(Qt5PrintSupport)
find_package(Qt5Concurrent)
find_package(Qt5Svg)
# Include Qt5 directories early in case another version of Qt exists on the system path.
# For example, on OSX, Qt4 may be installed on the framework paths and included first
# via the use of some other framework library.
include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Concurrent_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5OpenGL_INCLUDE_DIRS}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}
  )

if(MSVC)
  # Disable some compiler warnings.
  # 4267 - type conversion
  add_definitions("/wd4267")
endif(MSVC)

# Set include directories.
include_directories(.)

# List source files. Add additional directories here. Defaults to find C++ source files.
# Added to "SOURCES" variable.
list_source_files(SOURCES DIRS .)
list(APPEND SOURCES "${CONFIG_HEADER}")

# Ensure Visual Studio grouping matches that on disk.
group_source_by_dir(${SOURCES})

add_definitions("-DQWT_DLL -DQWT_MAKEDLL")

add_library(${TARGET} SHARED ${SOURCES})
set_target_properties(${TARGET} PROPERTIES MACOSX_RPATH ON)

# Link Qt5 modules
target_link_libraries(${TARGET} Qt5::Widgets Qt5::OpenGL Qt5::PrintSupport Qt5::Concurrent Qt5::Svg)

# Use per build type suffixes. For example, add "d" for debug builds.
target_auto_suffix(${TARGET})
