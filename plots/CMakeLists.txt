find_package(BISON QUIET)
find_package(FLEX QUIET)

set(PACKAGES ${QWT_PACKAGE})

if(BISON_FOUND AND FLEX_FOUND)
  set(BISON_AND_FLEX YES)
else(BISON_FOUND AND FLEX_FOUND)
  set(BISON_AND_FLEX NO)
endif(BISON_FOUND AND FLEX_FOUND)

# OpenGL, PrintSupport and SVG are not directly required for compiling, but required to run.
find_package(Qt5 CONFIG REQUIRED
  Widgets
)

# Set include directories.
configure_file(plotsconfig.in.h "${CMAKE_CURRENT_BINARY_DIR}/plotsconfig.h")

set(SOURCES
  expr/functionclean.cpp
  expr/functionclean.h
  expr/functiondefinition.cpp
  expr/functiondefinition.h
  expr/functionmavg.cpp
  expr/functionmavg.h
  expr/functionsimple.cpp
  expr/functionsimple.h
  expr/functionunwrap.cpp
  expr/functionunwrap.h
  expr/ocurves.ll
  expr/ocurvesparser.cpp
  expr/ocurvesparser.hpp
  expr/ocurvesscanner.cpp
  expr/ocurves.yy
  expr/plotbinaryoperator.cpp
  expr/plotbinaryoperator.h
  expr/plotbindinfo.cpp
  expr/plotbindinfo.h
  expr/plotbindingtracker.cpp
  expr/plotbindingtracker.h
  expr/plotbracketexpression.cpp
  expr/plotbracketexpression.h
  expr/plotconstant.cpp
  expr/plotconstant.h
  expr/plotexpressionarithmetic.h
  expr/plotexpressionbinddomain.cpp
  expr/plotexpressionbinddomain.h
  expr/plotexpression.cpp
  expr/plotexpression.h
  expr/plotexpressionparser.cpp
  expr/plotexpressionparser.h
  expr/plotfunction.cpp
  expr/plotfunction.h
  expr/plotfunctioninfo.h
  expr/plotfunctionregister.cpp
  expr/plotfunctionregister.h
  expr/plotfunctionresult.h
  expr/plotindexexpression.cpp
  expr/plotindexexpression.h
  expr/plotparseprivate.h
  expr/plotsample.cpp
  expr/plotsample.h
  expr/plotslice.cpp
  expr/plotslice.h
  expr/plotunaryoperator.cpp
  expr/plotunaryoperator.h
  plotinstance.cpp
  plotinstance.h
  plotinstancesampler.cpp
  plotinstancesampler.h
  plotsconfig.in.h
  plotsource.cpp
  plotsource.h
  plotutil.cpp
  plotutil.h
  refcountobject.h
  refcountptr.h
  timesampling.h
)

set(PUBLIC_HEADERS
  expr/functionclean.h
  expr/functiondefinition.h
  expr/functionmavg.h
  expr/functionsimple.h
  expr/functionunwrap.h
  expr/ocurvesparser.hpp
  expr/plotbinaryoperator.h
  expr/plotbindinfo.h
  expr/plotbindingtracker.h
  expr/plotbracketexpression.h
  expr/plotconstant.h
  expr/plotexpressionarithmetic.h
  expr/plotexpressionbinddomain.h
  expr/plotexpression.h
  expr/plotexpressionparser.h
  expr/plotfunction.h
  expr/plotfunctioninfo.h
  expr/plotfunctionregister.h
  expr/plotfunctionresult.h
  expr/plotindexexpression.h
  expr/plotparseprivate.h
  expr/plotsample.h
  expr/plotslice.h
  expr/plotunaryoperator.h
  plotinstance.h
  plotinstancesampler.h
  plotsource.h
  plotutil.h
  refcountobject.h
  refcountptr.h
  timesampling.h
)

set(DOC_HEADERS
  doc/docplots.h
)

# Add generated configuration headers to the public headers.
list(APPEND PUBLIC_HEADERS
  "${CMAKE_CURRENT_BINARY_DIR}/plotsconfig.h"
)

option(OPENCURVES_ALLOW_BISON_AND_FLEX "Allow regeneration of parser and scanner files if Bison and Flex are found." YES)

if(BISON_AND_FLEX AND OPENCURVES_ALLOW_BISON_AND_FLEX)
  message(STATUS "Building with Bison and Flex.")
  BISON_TARGET(OCurvesParser expr/ocurves.yy ${CMAKE_CURRENT_LIST_DIR}/expr/ocurvesparser.cpp)
  FLEX_TARGET(OCurvesScanner expr/ocurves.ll ${CMAKE_CURRENT_LIST_DIR}/expr/ocurvesscanner.cpp)
  ADD_FLEX_BISON_DEPENDENCY(OCurvesScanner OCurvesParser)
  set(BF_SOURCES ${CMAKE_CURRENT_LIST_DIR}/expr/ocurves.yy ${CMAKE_CURRENT_LIST_DIR}/expr/ocurves.ll ${BISON_OCurvesParser_OUTPUTS} ${FLEX_OCurvesScanner_OUTPUTS})
else(BISON_AND_FLEX AND OPENCURVES_ALLOW_BISON_AND_FLEX)
  if(OPENCURVES_ALLOW_BISON_AND_FLEX)
    if(NOT BISON_FOUND)
      message(STATUS "Missing Bison")
    endif(NOT BISON_FOUND)
    if(NOT FLEX_FOUND)
      message(STATUS "Missing Flex")
    endif(NOT FLEX_FOUND)
  endif(OPENCURVES_ALLOW_BISON_AND_FLEX)
  message(STATUS "Using pre-generated Bison and Flex files.")
  set(BF_SOURCES expr/ocurvesparser.cpp expr/ocurvesparser.hpp expr/ocurvesscanner.cpp)
endif(BISON_AND_FLEX AND OPENCURVES_ALLOW_BISON_AND_FLEX)

add_library(plots ${SOURCES} ${BF_SOURCES} ${PUBLIC_HEADERS} ${DOC_HEADERS})
target_include_directories(plots
  PUBLIC
    # Public include directories.
    # Include the CMAKE_CURRENT_BINARY_DIR during build to access the config and export headers.
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    # Also include the directory up for external targets correctly including the generated headers.
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    # Include the installation directory for import.
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(plots PUBLIC qwt::qwt Qt5::Widgets)

if(ALLOCTRACK_ENABLE)
  target_link_libraries(plots PUBLIC alloctrack)
endif(ALLOCTRACK_ENABLE)

# Setup installation.
# Binary installation
install(TARGETS plots EXPORT ${CMAKE_PROJECT_NAME}ConfigTargets
  LIBRARY DESTINATION lib COMPONENT Development
  ARCHIVE DESTINATION lib COMPONENT Development
  RUNTIME DESTINATION bin COMPONENT Runtime
  INCLUDES DESTINATION include COMPONENT Development
  # CONFIGURATIONS Release
)

# Install PDB files (MSVC)
if(MSVC AND BUILD_SHARED_LIBS)
  install(FILES $<TARGET_PDB_FILE:PLOTS> DESTINATION bin OPTIONAL Component Development)
endif(MSVC AND BUILD_SHARED_LIBS)

# Header installation
install(FILES ${PUBLIC_HEADERS} DESTINATION include/plots COMPONENT Development)

export(EXPORT ${CMAKE_PROJECT_NAME}ConfigTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigTargets.cmake"
  NAMESPACE opencurves::
)
install(EXPORT ${CMAKE_PROJECT_NAME}ConfigTargets
  FILE ${CMAKE_PROJECT_NAME}ConfigTargets.cmake
  NAMESPACE opencurves::
  DESTINATION ${ConfigPackageLocation}
)

# Setup folder display with the target for Visual Studio. This should always be done to match
# the on disk layout of the source files.
source_group("generated" FILES
  "${CMAKE_CURRENT_BINARY_DIR}/plotsconfig.h"
)
source_group("source" REGULAR_EXPRESSION ".*$")
source_group("source\\doc" REGULAR_EXPRESSION "/doc/.*$")
source_group("source\\expr" REGULAR_EXPRESSION "/expr/.*$")
