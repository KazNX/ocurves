option(ALLOCTRACK_SHARED "Build as a shared library?" YES)
option(ALLOCTRACK_DEBUG_SUFFIX "Append 'd' to the library name of the debug configuration" YES)
option(ALLOCTRACK_CONFIG_SUFFIXES "Append different suffixes to each library configuration. Overrides ALLOCTRACK_DEBUG_SUFFIX." YES)
option(ALLOCTRACK_TEST "Build test executable" NO)
option(ALLOCTRACK_DOXYGEN "Build Alloctrack doxygen html?" NO)

if(CMAKE_VERSION VERSION_LESS 2.8.3)
  get_filename_component(CMAKE_CURRENT_LIST_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
endif(CMAKE_VERSION VERSION_LESS 2.8.3)

configure_file(alloctrackconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/alloctrackconfig.h")
include_directories(. "${CMAKE_CURRENT_BINARY_DIR}")

if(MSVC)
  set(PLATFORM_DIR win)
else(MSVC)
  set(PLATFORM_DIR gcc)
endif(MSVC)

set(SOURCES alloctrack.cpp
            alloctrack.h
            alloctrackalloc.cpp
            alloctrackalloc.h
            "${CMAKE_CURRENT_BINARY_DIR}/alloctrackconfig.h"
            alloctrackdoxygen.h
            alloctrackoff.h
            debugbreak.h
            mutex.h
            stacktrace.h
            ${PLATFORM_DIR}/debugbreak.cpp
            ${PLATFORM_DIR}/mutex.cpp
            ${PLATFORM_DIR}/stacktrace.cpp
            )

set(PUBLIC_HEADERS alloctrack.h
                  alloctrackalloc.h
                  "${CMAKE_CURRENT_BINARY_DIR}/alloctrackconfig.h"
                  alloctrackdoxygen.h
                  alloctrackoff.h
                  debugbreak.h
                  stacktrace.h
                  )
set(DOXYGEN_DIRS "${CMAKE_CURRENT_LIST_DIR}")

SET(TARGET_NAME alloctrack)
set(ALLOCTRACK_TYPE STATIC)
if(ALLOCTRACK_SHARED)
  set(ALLOCTRACK_TYPE SHARED)
endif(ALLOCTRACK_SHARED)

add_library(${TARGET_NAME} ${ALLOCTRACK_TYPE} ${SOURCES})
set_target_properties(${TARGET_NAME} PROPERTIES MACOSX_RPATH ON)

if(MSVC)
  target_compile_definitions(${TARGET_NAME} PRIVATE alloctrack_EXPORTS)
endif(MSVC)
if(CMAKE_DL_LIBS)
  target_link_libraries(${TARGET_NAME} ${CMAKE_DL_LIBS})
endif(CMAKE_DL_LIBS)
# source_group(Source/${PLATFORM_DIR} REGULAR_EXPRESSION "${PLATFORM_DIR}/[^/]+$")
# source_group(Source FILES ${SOURCES})
if(MSVC)
  target_link_libraries(${TARGET_NAME} Dbghelp.dll)
else(MSVC)
  target_link_libraries(${TARGET_NAME} -ldl)
endif(MSVC)

function(get_config_suffix SUFFIX_VAR CONFIG)
  string(TOLOWER "${CONFIG}" CONFIGCMP)
  set(${SUFFIX_VAR} "" PARENT_SCOPE)
  if(ALLOCTRACK_DEBUG_SUFFIX OR ALLOCTRACK_CONFIG_SUFFIXES)
    if(CONFIGCMP MATCHES ".*debug$")
      set(${SUFFIX_VAR} d PARENT_SCOPE)
    endif()
  elseif(ALLOCTRACK_CONFIG_SUFFIXES)
    if(CONFIGCMP STREQUAL ".*relwithdebinfo$")
      set(${SUFFIX_VAR} r PARENT_SCOPE)
    elseif(CONFIGCMP STREQUAL ".*minsizerel$")
      set(${SUFFIX_VAR} m PARENT_SCOPE)
    endif()
  endif()
endfunction(get_config_suffix)

# Setup names of the generated libraries.
foreach(CONF ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
  get_config_suffix(SUFFIX ${CONF})
  string(TOUPPER ${CONF} CONF)
  set_target_properties(alloctrack PROPERTIES
                        ${CONF}_OUTPUT_NAME ${TARGET_NAME}${SUFFIX}
                       )
endforeach(CONF)

if(ALLOCTRACK_DOXYGEN)
  find_package(Doxygen QUIET)
  if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIR html)
    set(DOXYGEN_PROJECT AllocTrack)
    set(DOXYGEN_PROJECT_VERSION 1.0)
    foreach(dir ${DOXYGEN_DIRS})
      set(CMAKE_DOXYGEN_INPUT_LIST "${CMAKE_DOXYGEN_INPUT_LIST} \\\n  ${dir}")
    endforeach(dir)
    set(DOXYGEN_TEMPLATE "doxyfile.in")
    configure_file("${DOXYGEN_TEMPLATE}" "${CMAKE_CURRENT_BINARY_DIR}/doxyfile")
    get_filename_component(DOXYGEN_TEMPLATE_PATH "${DOXYGEN_TEMPLATE}" ABSOLUTE)
    add_custom_target(alloctrack-html
                      "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/doxyfile"
                      DEPENDS
                        "${DOXYGEN_TEMPLATE_PATH}"
                        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
                      )
    # # Do not build documentation as part of a debug build.
    # #set_target_properties(alloctrack-html PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE True)
    # install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/ DESTINATION html COMPONENT Development)
  endif(DOXYGEN_FOUND)
endif(ALLOCTRACK_DOXYGEN)

if(ALLOCTRACK_TEST)
  add_executable(alloctest ${EXE_FLAGS} alloctest.cpp)
  target_link_libraries(alloctest ${TARGET_NAME})
  set(EXE_TARGET alloctest)
endif(ALLOCTRACK_TEST)

# install(TARGETS ${TARGET_NAME} ${EXE_TARGET}
#         RUNTIME DESTINATION bin COMPONENT Development
#         LIBRARY DESTINATION lib COMPONENT Development
#         ARCHIVE DESTINATION lib COMPONENT Development)
# install(FILES ${PUBLIC_HEADERS} DESTINATION include COMPONENT Development)
