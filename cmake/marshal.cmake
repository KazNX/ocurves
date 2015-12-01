#==============================================================================
# CMake macros for marshalling files and creating marshalling rules.
# Most notably, supports setting up marshalling DLLs on Windows platforms.
#
# marshal_exe_dependencies(target)
#==============================================================================
include(CMakeParseArguments)

#==============================================================================
# List the set of available build configurations, mixed case. Defaults to RELEASE.
#==============================================================================
macro(m_list_configurations VAR)
  set(${VAR})
  if(DEFINED CMAKE_CONFIGURATION_TYPES)
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
      list(APPEND ${VAR} ${CONFIG})
    endforeach(CONFIG)
  endif(DEFINED CMAKE_CONFIGURATION_TYPES)
  list(APPEND ${VAR} ${CMAKE_BUILD_TYPE})
  if(NOT VAR)
    set(VAR RELEASE)
  endif(NOT VAR)
endmacro(m_list_configurations)

#==============================================================================
# marshal_exe_dependencies(customTargetName marshalTarget [PATHS path1 path2 ...])
#
# Generates a custom target to marshals DLL dependencies for built exectuables.
# The customTargetName specifies the custom target defined to marshal files,
# while marshalTarget specifies either an existing target, or an exectuable file
# path. If a target is specified, then it must be an executable target and is
# resolved to the path to the build exectuable. Depedencies are resolved for all
# exectuables and DLLs on the resolved path.
#
# Dependencies can be found on the path, or in any of the search paths specified
# after PATHS.
#
# Marshalling can be time consuming, thus it can be disabled by modifying
# MARSHAL_DISABLE to a false value in the cache.
#
# Note: this is only valid for WIN32 platforms and calls on other platforms do
# nothing.
#==============================================================================
function(marshal_exe_dependencies CUSTOM_TARGET_NAME MARSHAL_TARGET)
  if(NOT WIN32 OR MARSHAL_DISABLE)
    return()
  endif(NOT WIN32 OR MARSHAL_DISABLE)
  set(MARSHAL_DISABLE NO CACHE BOOL "Disable marshalling to make faster builds? Marshalling is generally only required for new executables or when DLL dependencies change.")

  CMAKE_PARSE_ARGUMENTS(MD "" "" "EXEDIRS;PATHS" ${ARGN})

  #----------------------------------------------
  # Setup
  # When not specified, we want MD_PATHS empty, not with "-NOTFOUND".
  if(NOT MD_PATHS)
    unset(MD_PATHS)
  endif(NOT MD_PATHS)

  # Define the output script.
  set(MARSHAL_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CUSTOM_TARGET_NAME}/cmake_marshal.cmake")

  # Will use CONFIGS if handling multiple configurations.
  set(CONFIGS)
  # Resolve specifying a target name vs. an exectuable path.
  if(TARGET "${MARSHAL_TARGET}")
    # Target name specified.
    get_target_property(TARGET_TYPE ${MARSHAL_TARGET} TYPE)
    if(NOT TARGET_TYPE STREQUAL "EXECUTABLE")
      message(SEND_ERROR " marshal_exe_dependencies() - ${MARSHAL_TARGET} is not an exectuable.")
      return()
    endif(NOT TARGET_TYPE STREQUAL "EXECUTABLE")
    if(CMAKE_CONFIGURATION_TYPES)
      foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} CONFIGUP)
        get_target_property(TARGET_DIR ${MARSHAL_TARGET} RUNTIME_OUTPUT_DIRECTORY_${CONFIGUP})
        get_target_property(TARGET_NAME ${MARSHAL_TARGET} OUTPUT_NAME_${CONFIGUP})
        if(NOT TARGET_DIR)
          set(TARGET_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIGUP}}")
        endif(NOT TARGET_DIR)
        set(TARGET_PATH_${CONFIGUP} "${TARGET_DIR}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
        list(APPEND CONFIGS ${CONFIGUP})
      endforeach(CONFIG)
    else(CMAKE_CONFIGURATION_TYPES)
      get_target_property(TARGET_DIR ${MARSHAL_TARGET} RUNTIME_OUTPUT_DIRECTORY)
      get_target_property(TARGET_NAME ${MARSHAL_TARGET} OUTPUT_NAME)
      if(NOT TARGET_PATH)
        set(TARGET_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
      endif(NOT TARGET_PATH)
      set(TARGET_PATH "${TARGET_DIR}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
    endif(CMAKE_CONFIGURATION_TYPES)
  else(TARGET "${MARSHAL_TARGET}")
    # Exectuable path specified directly.
    set(TARGET_PATH "${MARSHAL_TARGET}")
  endif(TARGET "${MARSHAL_TARGET}")

  if(NOT MD_EXEDIRS)
    set(MD_EXEDIRS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER ${CONFIG} CONFIGUP)
      list(APPEND MD_EXEDIRS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIGUP}}")
    endforeach(CONFIG)
  endif(NOT MD_EXEDIRS)

  #----------------------------------------------
  # Create script
  # Handle multi configuration builds
  if(CONFIGS)
      #-------------------------
      #-------------------------
    file(WRITE "${MARSHAL_FILE}" "include(BundleUtilities)
string(TOUPPER \"\${BUILD_TYPE}\" BUILD_TYPE_UPPER)
")
      #-------------------------
      #-------------------------
    foreach(CONFIG ${CONFIGS})
      unset(TARGET_PATH)
      if(TARGET_TYPE MATCHES "(EXECUTABLE)|(SHARED_LIBRARY)|(MODULE_LIBRARY)")
        get_target_property(TARGET_DIR ${MARSHAL_TARGET} RUNTIME_OUTPUT_DIRECTORY_${CONFIG})
        get_target_property(TARGET_NAME ${MARSHAL_TARGET} OUTPUT_NAME_${CONFIG})
        if(NOT TARGET_DIR)
          set(TARGET_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG}}")
        endif(NOT TARGET_DIR)
        set(TARGET_PATH "${TARGET_DIR}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
      endif(TARGET_TYPE MATCHES "(EXECUTABLE)|(SHARED_LIBRARY)|(MODULE_LIBRARY)")

      #-------------------------
      #-------------------------
      file(APPEND "${MARSHAL_FILE}" "
if(\"\${BUILD_TYPE_UPPER}\" MATCHES \"${CONFIG}\")
  fixup_bundle(\"${TARGET_PATH}\" \"\" \"${MD_PATHS}\")
endif(\"\${BUILD_TYPE_UPPER}\" MATCHES \"${CONFIG}\")
")
      #-------------------------
      #-------------------------
    endforeach(CONFIG)
  else(CONFIGS)

    # Single configuration build
    #-------------------------
    #-------------------------
    file(WRITE "${MARSHAL_FILE}" "
include(BundleUtilities)
fixup_bundle(\"${TARGET_PATH}\" \"\" \"${MD_PATHS}\")
")
  endif(CONFIGS)

  #----------------------------------------------
  # Define the target
  add_custom_target(${CUSTOM_TARGET_NAME} "${CMAKE_COMMAND}" ARGS -DBUILD_TYPE="\$\(CONFIGURATION\)" -P "${MARSHAL_FILE}")
endfunction(marshal_exe_dependencies)

#==============================================================================
# Resolve the lib corresponding to a dll or pdb corresponding to a dll.
#
# resolve_corresponding_file(VAR library_file search_extension)
#
# E.g., dll for lib: resolve_corresponding_file(DLL_FILE file.lib dll)
#
# E.g., pdb for dll: resolve_corresponding_file(PDB_FILE file.dll pdb)
#
# Search order:
# - Same directory
# - Replacing any "lib" directory in the path with "bin".
# - Up one directory and into a "bin" directory.
# - Up one directory and into a "dll" directory.
#==============================================================================
function(resolve_corresponding_file VAR DEPFILE FILEEXT)
  string(TOLOWER "${DEPFILE}" DEPFILELOWER)
  set(${VAR} "${VAR}-NOTFOUND" PARENT_SCOPE)

  if("${DEPFILELOWER}" MATCHES "\\.${FILEEXT}$")
    # Already a DLL
    set(${VAR} "${DEPFILE}" PARENT_SCOPE)
    return()
  endif("${DEPFILELOWER}" MATCHES "\\.${FILEEXT}$")

  # Build a list of file names to try
  set(TEST_FILES)

  # Try resolve a DLL for the current file.
  # Use string() to strip the extension, not get_filename_component in case of
  # odd extensions like "dir/file.1.dll", lwhich would give "dir/file"
  get_filename_component(DEPDIR "${DEPFILE}" DIRECTORY)
  get_filename_component(DEPNAME "${DEPFILE}" NAME_WE)

  # First try the same directory as the library.
  list(APPEND TEST_FILES "${DEPDIR}/${DEPNAME}.${FILEEXT}")

  # Try replacing "lib" directory with a "bin" directory
  string(REGEX REPLACE "(^|/)lib($|/)?" "\\1bin\\2" DEPDIR2 "${DEPDIR}")
  list(APPEND TEST_FILES "${DEPDIR2}/${DEPNAME}.${FILEEXT}")

  # Try up a directory, and into a bin directory.
  get_filename_component(DEPDIR "${DEPDIR}" DIRECTORY)
  list(APPEND TEST_FILES "${DEPDIR}/bin/${DEPNAME}.${FILEEXT}")
  # Also do a dll directory
  list(APPEND TEST_FILES "${DEPDIR}/dll/${DEPNAME}.${FILEEXT}")

  foreach(TESTFILE ${TEST_FILES})
    if(EXISTS "${TESTFILE}")
      set(${VAR} "${TESTFILE}" PARENT_SCOPE)
      return()
    endif(EXISTS "${TESTFILE}")
  endforeach(TESTFILE)
endfunction(resolve_corresponding_file)


#==============================================================================
# Add a list of dependency DLL and optionally PDB files for TARGET.
#
# add_dependencies_for_config(var target
#                             config
#                             PDB pdb_var
#                             FALLBACK config
#                            )
#
# config is one of: [Debug Release RelWithDebInfo MinSizeRel]
# PDB: Output variable for PDB file list.
# FALLBACK: Fallback to this configuration if config finds nothing.
#   E.g., use Release with MinSizeRel to use Release DLLs as a fallback.
#==============================================================================
function(add_dependencies_for_config VAR TARGET CONFIG)
  CMAKE_PARSE_ARGUMENTS(ADC "" "PDB;FALLBACK" "" ${ARGN})
  string(TOUPPER ${CONFIG} CONFIGUP)

  # List libraries with LINK_LIBRARIES[_CONFIG]
  get_target_property(DEPDENDENCIES_GENERAL ${TARGET} INTERFACE_LINK_LIBRARIES)
  get_target_property(DEPDENDENCIES ${TARGET} INTERFACE_LINK_LIBRARIES_${CONFIGUP})

  if(NOT DEPDENDENCIES AND ADC_FALLBACK)
    get_target_property(DEPDENDENCIES ${TARGET} LINK_LIBRARIES_${ADC_FALLBACK})
  endif(NOT DEPDENDENCIES AND ADC_FALLBACK)

  if(NOT DEPDENDENCIES AND NOT DEPDENDENCIES_GENERAL)
    return()
  endif(NOT DEPDENDENCIES AND NOT DEPDENDENCIES_GENERAL)

  # Merge lists.
  if(NOT DEPDENDENCIES)
    # Clear -NOTFOUND pattern.
    set(DEPDENDENCIES)
  endif(NOT DEPDENDENCIES)
  if(DEPDENDENCIES_GENERAL)
    list(APPEND DEPDENDENCIES ${DEPDENDENCIES_GENERAL})
  endif(DEPDENDENCIES_GENERAL)

  # message("get_target_property(DEPDENDENCIES ${TARGET} LINK_LIBRARIES_${CONFIGUP})")
  # message("DEPDENDENCIES: ${DEPDENDENCIES}")

  # Search for DLLs and PDBs
  if(ADC_PDB)
    set(${ADC_PDB})
  endif(ADC_PDB)
  set(${VAR})

  foreach(DEP ${DEPDENDENCIES})
    # - May be a library name or a target name
    # - For targets (or aliases such as Qt), use IMPORTED_LOCATION[_CONFIG] to resolve the dll.
    set(PDB_LOCATION)
    if(NOT DEP MATCHES "^\\$\\<.*$")
      if(EXISTS ${DEP})
        # Dependency is listed as a file. Resolve to a DLL file if possible.
        resolve_corresponding_file(DLL_LOCATION ${DEP} dll)
        if(ADC_PDB)
          resolve_corresponding_file(PDB_LOCATION ${DEP} pdb)
        endif(ADC_PDB)
      else(EXISTS ${DEP})
        # Resolve alias is dependency is a target.
        get_target_property(DLL_LOCATION ${DEP} IMPORTED_LOCATION_${CONFIGUP})
        # Try fallback.
        if(NOT DLL_LOCATION AND ADC_FALLBACK)
          get_target_property(DLL_LOCATION ${DEP} IMPORTED_LOCATION_${ADC_FALLBACK})
        endif(NOT DLL_LOCATION AND ADC_FALLBACK)
        if(DLL_LOCATION)
          # Recurse on the target.
          set(MORE_ARGS)
          if(ADC_PDB)
            set(MORE_ARGS PDB ${ADC_PDB})
          endif(ADC_PDB)
          if(ADC_FALLBACK)
            set(MORE_ARGS FALLBACK ${ADC_FALLBACK})
          endif(ADC_FALLBACK)
          add_dependencies_for_config(${VAR} ${DEP} ${CONFIG} ${MORE_ARGS} ${ADC_UNPARSED_ARGUMENTS})

          if(ADC_PDB)
            resolve_corresponding_file(PDB_LOCATION ${DLL_LOCATION} pdb)
          endif(ADC_PDB)
        endif(DLL_LOCATION)
      endif(EXISTS ${DEP})
    else()
      # message("NOT FOR ${DEP}")
    endif()

    if(DLL_LOCATION)
      list(APPEND ${VAR} "${DLL_LOCATION}")
    endif(DLL_LOCATION)

    if(ADC_PDB AND PDB_LOCATION)
      list(APPEND ${ADC_PDB} "${PDB_LOCATION}")
    endif(ADC_PDB AND PDB_LOCATION)
  endforeach(DEP)

  # Copy DLLs to parent scope
  set(${VAR} "${${VAR}}" PARENT_SCOPE)
  # Copy PDBs to parent scope
  if(ADC_PDB)
    set(${ADC_PDB} "${${ADC_PDB}}" PARENT_SCOPE)
  endif(ADC_PDB)
endfunction(add_dependencies_for_config)


#==============================================================================
# Marshals dependencies (DLLs) for TARGET to the TARGET build directory.
# For DLLs and executables only and only on WIN32
# 
# marshal_dependencies2(target [DLLPATH] [LIBS])
#==============================================================================
function(marshal_dependencies2 TARGET)
  CMAKE_PARSE_ARGUMENTS(MARSHAL "" "DLLPATH" "LIBS" ${ARGN})

  if(NOT WIN32)
    return()
  endif(NOT WIN32)

  m_list_configurations(CONFIG_LIST)
  foreach(CONFIG ${CONFIG_LIST})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(FALLBACK_OPT)
    set(FALLBACK_ARG)
    if(NOT "${CONFIG_UPPER}" STREQUAL "DEBUG")
      set(FALLBACK_OPT FALLBACK)
      set(FALLBACK_ARG RELEASE)
    endif(NOT "${CONFIG_UPPER}" STREQUAL "DEBUG")
    add_dependencies_for_config(DLLS_${CONFIG_UPPER} ${TARGET} ${CONFIG_UPPER} PDB DLLS_${CONFIG_UPPER} ${FALLBACK_OPT} ${FALLBACK_ARG})
  endforeach(CONFIG)

  # Marshal to the bin directory.
  set(SUBDIR)
  if(MARSHAL_DLLPATH)
    set(SUBDIR "/${MARSHAL_DLLPATH}")
  endif(MARSHAL_DLLPATH)

  foreach(CONFIG ${CONFIG_LIST})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    get_target_property(DEST_DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER})
    if(NOT DEST_DIR)
      get_target_property(DEST_DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY)
    endif(NOT DEST_DIR)

    if(NOT DEST_DIR)
      message(SEND_ERROR "Unable to resolve runtime output directory for ${TARGET}.")
      return()
    endif(NOT DEST_DIR)

    foreach(DLL_FILE ${DLLS_${CONFIG_UPPER}})
      file(COPY "${DLL_FILE}" DESTINATION "${DEST_DIR}${SUBDIR}")
    endforeach(DLL_FILE)
  endforeach(CONFIG)
endfunction(marshal_dependencies2)
