#==============================================================================
# Windows specific extensions.
#==============================================================================

include(CMakeParseArguments)

set(SHARED_LIB_EXPORT "__declspec(dllexport)")
set(SHARED_LIB_IMPORT "__declspec(dllimport)")
set(SHARED_LIB_HIDDEN " ")

# Windows setup is different. "Library" covers DLLs.
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Visual Studio 2010+ has an issue whereby it continually considers the ZERO_CHECK and ALL_BUILD
# projects  of date. Visual Studio reports the reason (indirectly) as being because
#   "<build-folder>/CMakeFiles/ZERO_CHECK" and ALL_BUILD are missing. Creating these files
#  circumvents the issue. This macro does so, but only for MSVC projects.
function(fix_msvc_rebuild)
  if(MSVC)
    set(ZERO_CHECK_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/ZERO_CHECK")
    set(ALL_BUILD_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/ALL_BUILD")
    if(NOT EXISTS "${ZERO_CHECK_FILE}")
      file(WRITE "${ZERO_CHECK_FILE}" "Dummy file to meet ZERO_CHECK dependency checking.")
    endif(NOT EXISTS "${ZERO_CHECK_FILE}")
    if(NOT EXISTS "${ALL_BUILD_FILE}")
      file(WRITE "${ALL_BUILD_FILE}" "Dummy file to meet ALL_BUILD dependency checking.")
    endif(NOT EXISTS "${ALL_BUILD_FILE}")
    # Alternative path description or create as directory instead of file.
    # These are left here in case there is some issue with using a file or the
    # variables used to generate the path name.
    #file(WRITE "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/ZERO_CHECK" "Dummy file to meet ZERO_CHECK dependency checking.")
    #file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/ZERO_CHECK")
  endif(MSVC)
endfunction(fix_msvc_rebuild)

fix_msvc_rebuild()

macro(enable_release_debug_info)
  if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "/debug ${CMAKE_MODULE_LINKER_FLAGS_RELEASE}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/debug ${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/debug ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
  endif(MSVC)
endmacro(enable_release_debug_info)

