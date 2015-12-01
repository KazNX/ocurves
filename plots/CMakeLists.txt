set(TARGET plots)

find_package(BISON QUIET)
find_package(FLEX QUIET)

set(PACKAGES ${QWT_PACKAGE})

if(ALLOCTRACK_ENABLE)
  list(APPEND PACKAGES alloctrack)
endif(ALLOCTRACK_ENABLE)

if(BISON_FOUND AND FLEX_FOUND)
  set(BISON_AND_FLEX YES)
else(BISON_FOUND AND FLEX_FOUND)
  set(BISON_AND_FLEX NO)
endif(BISON_FOUND AND FLEX_FOUND)

# OpenGL, PrintSupport and SVG are not directly required for compiling, but required to run.
find_package(Qt5Widgets)

# Set include directories.
target_config_file(${TARGET} plotsconfig.h VAR CONFIG_HEADER)
include_directories(. "${CMAKE_CURRENT_BINARY_DIR}")
include_directories(${Qt5Widgets_INCLUDE_DIRS})

# List source files. Add additional directories here. Defaults to find C++ source files.
# Added to "SOURCES" variable.
list_source_files(SOURCES DIRS . expr doc)

make_option(OCURVES_ALLOW_BISON_AND_FLEX "Allow regeneration of parser and scanner files if Bison and Flex are found." YES)

if(BISON_AND_FLEX AND OCURVES_ALLOW_BISON_AND_FLEX)
  message(STATUS "Building with Bison and Flex.")
  BISON_TARGET(OCurvesParser expr/ocurves.yy ${CMAKE_CURRENT_LIST_DIR}/expr/ocurvesparser.cpp)
  FLEX_TARGET(OCurvesScanner expr/ocurves.ll ${CMAKE_CURRENT_LIST_DIR}/expr/ocurvesscanner.cpp)
  ADD_FLEX_BISON_DEPENDENCY(OCurvesScanner OCurvesParser)
  set(BF_SOURCES ${CMAKE_CURRENT_LIST_DIR}/expr/ocurves.yy ${CMAKE_CURRENT_LIST_DIR}/expr/ocurves.ll ${BISON_OCurvesParser_OUTPUTS} ${FLEX_OCurvesScanner_OUTPUTS})
else(BISON_AND_FLEX AND OCURVES_ALLOW_BISON_AND_FLEX)
  message(STATUS "Using pre-generated Bison and Flex files.")
  set(BF_SOURCES expr/ocurvesparser.cpp expr/ocurvesparser.hpp expr/ocurvesscanner.cpp)
endif(BISON_AND_FLEX AND OCURVES_ALLOW_BISON_AND_FLEX)

# Ensure Visual Studio grouping matches that on disk.
group_source_by_dir(${SOURCES} ${BF_SOURCES})

include_packages(${PACKAGES})
add_library(${TARGET} ${SOURCES} ${BF_SOURCES})
link_packages(${TARGET} ${PACKAGES})

# Link Qt5 modules
target_link_libraries(${TARGET} Qt5::Widgets)
#target_link_libraries(${TARGET} Qt5::Widgets Qt5::OpenGL Qt5::PrintSupport Qt5::Concurrent Qt5::Svg)

add_doxygen_directories(${SOURCES} PARENT_SCOPE)
