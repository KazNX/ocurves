#==============================================================================
# Contains various helper and utility functions.
#==============================================================================
include(CMakeParseArguments)

#==============================================================================
# list_replace([REGEX] listvar <match> <replace>)
# Replaces whole items in the given listvar.
#==============================================================================
function(list_replace)
  CMAKE_PARSE_ARGUMENTS(LR "REGEX" "" "" ${ARGN})

  list(GET LR_UNPARSED_ARGUMENTS 0 LR_LIST)
  list(GET LR_UNPARSED_ARGUMENTS 1 LR_MATCH)
  list(GET LR_UNPARSED_ARGUMENTS 2 LR_REPLACE)

  if(LR_REGEX)
    set(LR_REGEX REGEX)
  else(LR_REGEX)
    set(LR_REGEX)
  endif(LR_REGEX)

  set(REPLACED_LIST)
  foreach(ITEM ${${LR_LIST}})
    string(${LR_REGEX} REPLACE "${LR_MATCH}" "${LR_REPLACE}" ITEMX "${ITEM}")
    list(APPEND REPLACED_LIST "${ITEMX}")
  endforeach(ITEM)

  set(${LR_LIST} ${REPLACED_LIST} PARENT_SCOPE)
endfunction(list_replace)

#==============================================================================
# SET_BUILD_RULE(item
#                [INPUT input]
#                OUTPUT output
#                RULE customRule
#                [COMMENT comment])
#
# Setup a custom build rule for item. The item must be an item in the project,
# but must not be a source file or other item that already has a well known
# build rule.
#
# item - The project item to define a custom build rule for. Must match the
#     item as it is added to the executable/library.
# INPUT input - (Optional) Specifies full the input file path. Defaults to
#     the value of ${item}.
# OUTPUT output - Specifies the full directory or path of the generated output.
# RULE customRule - Lists the command string to execute as the custom build
#     rule. The input file may be specified using INPUT_FILE and the output
#     using OUTPUT_FILE. See examples.
# COMMENT comment - (Optional) Specifies the comment to show in the build
#     process for this build rule.
#
# Examples:
#  SET_BUILD_RULE(src/marshalled.txt
#                 INPUT ${PROJECT_SOURCE_DIR}/src/marshalled.txt
#                 OUTPUT ${PROJECT_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/marshalled.txt
#                 RULE "${CMAKE_COMMAND}" -E copy_if_different INPUT_FILE OUTPUT_FILE
#                 COMMENT "Copying ${item}...")
# This sets up a marshalling build rule for src/marshalled.txt. This file is in
# a subdirectory of the project called 'src'. INPUT is specified as the full
# path to marshalled.txt, OUTPUT is specified as a full output path. The rule
# uses the CMake executable to perform a copy_if_different operation give
# INPUT_FILE and OUTPUT_FILE as the source and destination respectively (maps
# to INPUT and OUTPUT).
#==============================================================================
function(set_rule item)
  CMAKE_PARSE_ARGUMENTS(SR "" "INPUT;OUTPUT;COMMENT" "RULE;DEPENDS" ${ARGN})

  if(NOT SR_INPUT)
    set(SR_INPUT ${item})
  endif(NOT SR_INPUT)

  if(NOT SR_OUTPUT)
    message(SEND_ERROR "SET_BUILD_RULE(${item} ${ARGN}) - OUTPUT not specified.")
  endif(NOT SR_OUTPUT)

  get_filename_component(absoluteInput ${SR_INPUT} ABSOLUTE)
  list_replace(SR_RULE "INPUT_FILE" ${absoluteInput})
  list_replace(SR_RULE "OUTPUT_FILE" ${SR_OUTPUT})

  if(NOT SR_RULE)
    message(SEND_ERROR "set_rule(${item} ${ARGN}) - RULE not specified.")
  endif(NOT SR_RULE)

  if(SR_COMMENT)
    if(SR_DEPENDS)
      add_custom_command(OUTPUT ${SR_OUTPUT}
                         MAIN_DEPENDENCY ${item}
                         COMMAND ${SR_RULE}
                         COMMENT ${SR_COMMENT}
                         DEPENDS ${SR_DEPENDS}
                         )
    else(SR_DEPENDS)
      add_custom_command(OUTPUT ${SR_OUTPUT}
                           MAIN_DEPENDENCY ${item}
                           COMMAND ${SR_RULE}
                           COMMENT ${SR_COMMENT}
                           ${SR_DEPENDS}
                           )
    endif(SR_DEPENDS)
  else(SR_COMMENT)
    if(SR_DEPENDS)
      add_custom_command(OUTPUT ${SR_OUTPUT}
                         MAIN_DEPENDENCY ${item}
                         COMMAND ${SR_RULE}
                         DEPENDS ${SR_DEPENDS}
                         )
    else(SR_DEPENDS)
      add_custom_command(OUTPUT ${SR_OUTPUT}
                           MAIN_DEPENDENCY ${item}
                           COMMAND ${SR_RULE}
                           ${SR_DEPENDS}
                           )
    endif(SR_DEPENDS)
  endif(SR_COMMENT)
endfunction(set_rule)


#==============================================================================
# ADD_MARSHAL_BUILD_RULE(item [SOURCE sourcePath] [TARGET targetPath])
#
# Defines a build step to copy a file to a target directory. The file must be
# a member of the project being defined.
#
# item - The project item to associate the marshal rule with. This must match
#   how the item is defined when added to the project.
# SOURCE sourcePath - (Optional) Specifies the full source path of the file to
#   marshal. Defaults to matching {item}.
# TARGET targetPath - (Optional) Specifies the target directory or target path
#   to marshal to. Defaults to ${PROJECT_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}.
#==============================================================================
function(add_marshal_rule item)
  CMAKE_PARSE_ARGUMENTS(AMR "" "SOURCE;TARGET" "" ${ARGN})

  if(NOT item)
    message(SEND_ERROR "ADD_MARSHAL_BUILD_RULE(${item} ${ARGN}): item not set.")
  endif(NOT item)

  if(NOT AMR_SOURCE)
    set(AMR_SOURCE ${item})
  endif(NOT AMR_SOURCE)

  if(NOT AMR_TARGET)
    get_filename_component(fileName ${item} NAME)
    # Marshal using ${CMAKE_CFG_INTDIR} if output directories differ. Makes a lot of assumptions.
    if("${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}" STREQUAL "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE}")
      set(AMR_TARGET ${PROJECT_BINARY_DIR}/bin/${fileName})
    else("${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}" STREQUAL "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE}")
      set(AMR_TARGET ${PROJECT_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/${fileName})
    endif("${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}" STREQUAL "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE}")
  endif(NOT AMR_TARGET)

  set_rule(${item} INPUT ${AMR_SOURCE} OUTPUT ${AMR_TARGET}
                 RULE "${CMAKE_COMMAND}" -E copy_if_different INPUT_FILE OUTPUT_FILE
                 COMMENT "Copying ${item}...")
endfunction(add_marshal_rule)


#===============================================================================
# LIST_SOURCE_FILES(filesVar
#                   DIRS dir1, dir2, ...
#                   [EXTS ext1, ext2, ...])
#
# Grep the given directories for source files. Add them to a list in a structure
# that allows sorting into directories for generation of folder structures in
# Visual Studio.
#
# EXT specifies the file extensions to add. The default values are "cpp", "inl"
# and "h".
#===============================================================================
function(list_source_files filesVar)
  CMAKE_PARSE_ARGUMENTS(LSF "" "" "DIRS;EXTS" ${ARGN})

  if(NOT LSF_EXTS)
    set(LSF_EXTS cpp;inl;h;hpp)
  endif(NOT LSF_EXTS)

  set(foundSources)
  set(source_files)
  foreach(dir ${LSF_DIRS})
    foreach(ext ${LSF_EXTS})
      #file(GLOB_RECURSE source_files "${dir}/*.${ext}")
      if(dir STREQUAL .)
        file(GLOB source_files "*.${ext}")
      else()
        file(GLOB source_files "${dir}/*.${ext}")
      endif()
      list(APPEND foundSources ${source_files})
    endforeach(ext)
  endforeach(dir)

  set(${filesVar} ${${filesVar}} ${foundSources} PARENT_SCOPE)
endfunction(list_source_files)

#===============================================================================
# Tests a configuration name to see if it is a debug configuration.
# By default, the only debug configuration is "DEBUG" unless the global property
# DEBUG_CONFIGURATIONS is specified. This function handles both cases, with
# the property taking precedence.
#
# Usage:
#   is_debug_config(<config> <variable>)
#
# The value of <variable> is set to YES or NO depending on whether or not
# <config> matches a debug configuration name. This test is case insensitive.
#===============================================================================
function(is_debug_config config var)
  get_property(DEBUG_CONFIGS GLOBAL PROPERTY DEBUG_CONFIGURATIONS)
  
  string(TOLOWER ${config} CONFIG_LOWER)
  if(DEBUG_CONFIGS)
    set(DBG_MATCH NO)
    foreach(CFG DEBUG_CONFIGS)
      string(TOLOWER ${CFG} CFG_LOWER)
      if(CFG_LOWER STREQUAL CONFIG_LOWER)
        set(DBG_MATCH YES)
      endif(CFG_LOWER STREQUAL CONFIG_LOWER)
    endforeach(CFG)
    set(${var} ${DBG_MATCH} PARENT_SCOPE)
  else(DEBUG_CONFIGS)
    if(${CONFIG_LOWER} STREQUAL "debug")
      set(${var} YES PARENT_SCOPE)
    else(${CONFIG_LOWER} STREQUAL "debug")
      set(${var} NO PARENT_SCOPE)
    endif(${CONFIG_LOWER} STREQUAL "debug")
  endif(DEBUG_CONFIGS)
endfunction(is_debug_config)

#===============================================================================
# Make an option, but use an existing variable value if present.
# Functions like the CMake command option(), but first checks if there is an
# existing variable with the same name as the option. If so, it uses that value
# as the default.
#
# Usage: 
#   make_option(<name> <description> <default> [NUMERIC])
# NUMERIC: When present, the variable <name> is set in the parent scope as
#   [0, 1], rather than [TRUE, FLASE, ON, OFF, YES, NO]. This is useful for use
#   of option variables in macro expansion for C/C++ code.
#   See make_bool_numeric()
#===============================================================================
function(make_option OPT_NAME OPT_DESC OPT_DEFAULT)
  CMAKE_PARSE_ARGUMENTS(MO "NUMERIC" "" "" ${ARGN})
  set(OPT_DEFAULT_VALUE ${OPT_DEFAULT})
  if(DEFINED ${OPT_NAME})
    set(OPT_DEFAULT_VALUE ${${OPT_NAME}})
  endif()
  option(${OPT_NAME} ${OPT_DESC} ${OPT_DEFAULT_VALUE})
  if(MO_NUMERIC)
    make_bool_numeric(${OPT_NAME} ${${OPT_NAME}})
    set(${OPT_NAME} ${${OPT_NAME}} PARENT_SCOPE)
    set(${OPT_NAME} ${${OPT_NAME}} PARENT_SCOPE)
  endif(MO_NUMERIC)
endfunction(make_option)


#===============================================================================
# Converts any Boolean style variable test into a binary, numeric value [0, 1].
# This is useful for setting converting CMake variables into preprocessor
# switches in configuration header files.
#
# Example:
# CMake script:
#   # Make an option for USE_WIN_LEAN_AND_MEAN. Comes out as either YES/NO or
#   # ON/OFF.
#   option(USE_WIN_LEAN_AND_MEAN "Define WIN32_LEAN_AND_MEAN?" YES)
#   # Convert to 0/1
#   make_bool_numeric(USE_WIN_LEAN_AND_MEAN USE_WIN_LEAN_AND_MEAN)
#   # Configure config.h.in
#   file(configure config.h.in config.h)
#
# config.h.in:
# #ifndef CONFIG_H
# #define CONFIG_H
#
# // @USE_WIN_LEAN_AND_MEAN@ is replaced with 0 or 1.
# #define USE_WIN_LEAN_AND_MEAN @USE_WIN_LEAN_AND_MEAN@
#
# #if USE_WIN_LEAN_AND_MEAN
# #define WIN32_LEAN_AND_MEAN
# #endif // USE_WIN_LEAN_AND_MEAN
#
# #endif CONFIG_H
#===============================================================================
function(make_bool_numeric BOOL_VAR)
  if(ARGC GREATER 0)
    if(${BOOL_VAR})
      set(${ARGV0} 1 PARENT_SCOPE)
    else(${BOOL_VAR})
      set(${ARGV0} 0 PARENT_SCOPE)
    endif(${BOOL_VAR})
  else(ARGC GREATER 0)
    if(${BOOL_VAR})
      set(${BOOL_VAR} 1 PARENT_SCOPE)
    else(${BOOL_VAR})
      set(${BOOL_VAR} 0 PARENT_SCOPE)
    endif(${BOOL_VAR})
  endif(ARGC GREATER 0)
endfunction(make_bool_numeric)

#===============================================================================
# Marshal a file to the runtime directory or directories.
# This handles marshalling the file to multiple directories when using multiple
# build configurations and CSF_PER_CONF_DIRS is true.
#
# Specifying debug or optimized marshals the file only to the debug
# or release configuration directories respectively.
#
# Usage:
#   marshal_file([TARGET target-name] [SUB_DIR sub-dir] [debug|optimized|general] <file-path>)
#
# TARGET specifies the target name to marshal against. When specified, the file
# is marshalled using the current target's RUNTIME_OUTPUT_DIRECTORY rather than
# the global CMAKE_RUNTIME_OUTPUT_DIRECTORY.
#
# SUB_DIR allows the file to be marshalled to the specified sub directory.
#===============================================================================
function(marshal_file)
  #message("marshal_file(${ARGN})")
  # Handle multi- and single configuration builds.
  if(CMAKE_CONFIGURATION_TYPES)
    set(MULTI_FLAG "MULTI")
    if(NOT CSF_PER_CONF_DIRS)
      # Don't specify the "MULTI" option if not using per configuration directories.
      set(MULTI_FLAG)
    endif(NOT CSF_PER_CONF_DIRS)
    # Iterate the build targets.
    foreach(CONF ${CMAKE_CONFIGURATION_TYPES})
      marshal_file_for_config(${CONF} ${MULTI_FLAG} ${ARGN})
    endforeach(CONF)
  else(CMAKE_CONFIGURATION_TYPES)
    marshal_file_for_config("${CMAKE_BUILD_TYPE}" ${ARGN})
  endif(CMAKE_CONFIGURATION_TYPES)
endfunction(marshal_file)


#===============================================================================
# Helper for marshal_file (internal use) to avoid code duplication.
#===============================================================================
function(marshal_file_for_config CONFIG)
  CMAKE_PARSE_ARGUMENTS(MFFC "debug;optimized;general;MULTI" "TARGET;SUB_DIR" "" ${ARGN})

  # Validate configuration.
  is_debug_config(${CONFIG} IS_DEBUG)
  if(MFFC_debug AND NOT ${IS_DEBUG})
    return()
  endif(MFFC_debug AND NOT ${IS_DEBUG})
  if(MFFC_optimized AND ${IS_DEBUG})
    return()
  endif(MFFC_optimized AND ${IS_DEBUG})

  # Resolve the destination directory using the first successful rule as follows:
  #   1. TARGET argument specified, use TARGET property RUNTIME_OUTPUT_DIRECTORY_${CONFIGU}
  #   2. TARGET arguments specified, use TARGET PROPERTY RUNTIME_OUTPUT_DIRECTORY
  #   3. Use CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIGU}
  #   4. Use CMAKE_RUNTIME_OUTPUT_DIRECTORY
  #
  # Then if using 2 or 4 and the MULTI argument is specified, append
  # the build configuration name as a subdirectory.
  #
  # Add ${MFFC_SUB_DIR} if specified.

  set(DEST_DIR)
  set(CONF_DIR)
  # Resolve destination path.
  string(TOUPPER "${CONFIG}" CONFIGU)
  if(MFFC_TARGET)
    get_target_property(DEST_DIR ${MFFC_TARGET} "RUNTIME_OUTPUT_DIRECTORY_${CONFIGU}")
    if(NOT DEST_DIR)
      get_target_property(DEST_DIR ${MFFC_TARGET} "RUNTIME_OUTPUT_DIRECTORY")
      if(DEST_DIR)
        set(CONF_DIR "${CONFIG}")
      endif(DEST_DIR)
    endif(NOT DEST_DIR)
  endif(MFFC_TARGET)

  if(NOT DEST_DIR)
    set(DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIGU}}")
    if(NOT DEST_DIR)
      set(DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
      set(CONF_DIR "${CONFIG}")
    endif(NOT DEST_DIR)
  endif(NOT DEST_DIR)

  if(MFFC_MULTI AND CONF_DIR)
    set(DEST_DIR "${DEST_DIR}/${CONF_DIR}")
  endif(MFFC_MULTI AND CONF_DIR)

  if(MFFC_SUB_DIR)
    set(DEST_DIR "${DEST_DIR}/${MFFC_SUB_DIR}")
  endif(MFFC_SUB_DIR)

  # Have destination DEST_DIR.

  # Resolve source. Append 'NOTFOUND' in case MFFC_UNPARSED_ARGUMENTS is empty.
  list(APPEND MFFC_UNPARSED_ARGUMENTS NOTFOUND)
  list(GET MFFC_UNPARSED_ARGUMENTS 0 SOURCE_PATH)

  if(NOT SOURCE_PATH)
    return()
  endif(NOT SOURCE_PATH)

  # This is a debug configuration. Copy the dll to this path.
  if(EXISTS "${SOURCE_PATH}")
    file(COPY "${SOURCE_PATH}" DESTINATION "${DEST_DIR}")
  endif(EXISTS "${SOURCE_PATH}")
endfunction(marshal_file_for_config)


#===============================================================================
# Dumps the list of local CMake variables and their values.
#===============================================================================
function(dump_variables)
  message(STATUS "###########################################")
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
      message(STATUS "${_variableName}=${${_variableName}}")
  endforeach(_variableName)
  message(STATUS "###########################################")
endfunction(dump_variables)


#===============================================================================
# Invokes find_package(${PACKAGE} ${ARGN}), but only if an attempt to find the
# package has yet to be made. This can save on some log span and repeated
# searching.
#
# This relies on find_package() scripts setting ${PACKAGE}_FOUND and will only
# invoke find_package() if ${PACKAGE}_FOUND is not defined (found or not)
#
# Take care using this if specific components are required. In this case, the
# the components will only be present if the previous search included these.
#===============================================================================
macro(find_package_if PACKAGE)
  if(NOT DEFINED ${PACKAGE}_FOUND)
    find_package(${PACKAGE} ${ARGN})
  endif(NOT DEFINED ${PACKAGE}_FOUND)
endmacro(find_package_if)

#===============================================================================
# Creates source filters to group files by subdirectory. This affects IDE
# environments such as Visual Studio.
#
# Usage:
#   group_source_by_dir(file1 file2 ... fileN)
#===============================================================================
function(group_source_by_dir)
  if (group_source_by_dir_DBG)
    message("Files: ${ARGN}")
  endif (group_source_by_dir_DBG)
  extract_directories(_dirs "${ARGN}")
  #replace_in_list(_dirs REPLACE "${CMAKE_CURRENT_LIST_DIR}" ".")

  if(_dirs)
    # Build source groups for IDE environments such as Visual Studio.
    foreach(_dir ${_dirs})
      if (group_source_by_dir_DBG)
        message("_dir: ${_dir}")
      endif (group_source_by_dir_DBG)
      if(_dir STREQUAL .)
        if (group_source_by_dir_DBG)
          message("Group is .")
        endif (group_source_by_dir_DBG)
        set(_group_re "${CMAKE_CURRENT_LIST_DIR}/[^/]+")
        set(_groupFilter)
      else(_dir STREQUAL .)
        set(_group_re ${_dir}/[^/]+)
        string(REPLACE "${CMAKE_CURRENT_LIST_DIR}" "" _groupFilter "${_dir}")
        string(REPLACE "/" "\\" _groupFilter "${_groupFilter}")
        set(_groupFilter "${_groupFilter}")
      endif(_dir STREQUAL .)

      # Strip "\.\" pattern
      string(REPLACE "\\.\\" "\\" _groupFilter "${_groupFilter}")
      # Ensure leading backslash.
      if(NOT "${_groupFilter}" MATCHES "^\\\\.*")
        set(_groupFilter "\\${_groupFilter}")
      endif(NOT "${_groupFilter}" MATCHES "^\\\\.*")

      if (group_source_by_dir_DBG)
        message("Source${_groupFilter} -> ${_group_re}$")
      endif (group_source_by_dir_DBG)
      source_group("Source${_groupFilter}" ${_group_re}$)
    endforeach(_dir)
  endif(_dirs)
endfunction(group_source_by_dir)

#==============================================================================
# Sets up source groups (for Visual Studio) collating generated Qt files.
#
# This creates the following source group structure:
# + Generated
# +-- moc : for moc files, including any automoc file.
# +-- ui  : for generated UI files (ui_<class>.h)
# +-- res : for generated resource files (res_<name>)
#==============================================================================
function(group_qt_sources)
  source_group("Generated\\ui" "^(.*[\\\\/])?ui_.*\\.h$")
  source_group("Generated\\qrc" "^(.*[\\\\/])?qrc_.*\\.cpp$")
  source_group("Generated\\moc" "^((.*[\\\\/])?moc_.*)|(.*_automoc\\.cpp)$")
endfunction(group_qt_sources)

#===============================================================================
# Adjust compiler/linker flags.
#===============================================================================
function(mod_compile_flags config add remove)
  CMAKE_PARSE_ARGUMENTS(MCF "" "" "ADD;REMOVE" ${ARGN})

  string(TOUPPER varCFlags "CMAKE_C_FLAGS_${config}")
  string(TOUPPER varCXXFlags "CMAKE_C_FLAGS_${config}")

  foreach(flag ${MCF_REMOVE})
    string(REGEX REPLACE "${flag}" "" ${varCFlags} ${${varCFlags}})
    string(REGEX REPLACE "${flag}" "" ${varCXXFlags} ${${varCXXFlags}})
  endforeach(flag)

  foreach(flag ${MCF_ADD})
    if(NOT ${varCFlags} MATCHES "${flag}")
      set(${varCFlags} "${${varCFlags}} ${flag}")
    endif(NOT ${varCFlags} MATCHES "${flag}")
    if(NOT ${varCXXFlags} MATCHES "${flag}")
      set(${varCXXFlags} "${${varCXXFlags}} ${flag}")
    endif(NOT ${varCXXFlags} MATCHES "${flag}")
  endforeach(flag)

  set(${varCFlags} "${${varCFlags}}" CACHE STRING "C Compiler flags")
  set(${varCXXFlags} "${${varCXXFlags}}" CACHE STRING "CXX Compiler flags")

  set(${varCFlags} "${${varCFlags}}" PARENT_SCOPE)
  set(${varCXXFlags} "${${varCXXFlags}}" PARENT_SCOPE)
endfunction(mod_compile_flags)

#===============================================================================
# Filters a list of files to extract a list of directories containing those
# files. The result is written to ${OUT_VAR}.
#
# Usage:
#   extract_directories(<out-var> file1 file2 ... fileN)
#===============================================================================
function(extract_directories OUT_VAR)
  set(_dirs)
  foreach(_file ${ARGN})
    get_filename_component(_dir "${_file}" PATH)
    file(TO_CMAKE_PATH "${_dir}" _dir)
    list(APPEND _dirs "${_dir}")
  endforeach(_file)

  # Remove duplicates
  list(REMOVE_DUPLICATES _dirs)
  set(${OUT_VAR} "${_dirs}" PARENT_SCOPE)
endfunction(extract_directories OUT_VAR)

function(replace_in_list LIST REPLACE MATCH_STR REPLACE_STR)
  set(_list)
  foreach(_var "${${LIST}}")
    string(${REPLACE} "${MATCH_STR}" "${REPLACE_STR}" _var "${_var}")
    list(APPEND _list "${_var}")
  endforeach(_var)

  set(${LIST} ${_list} PARENT_SCOPE)
endfunction(replace_in_list)

#==============================================================================
# Get the directory containing a library catering for framework libraries.
#
# For a library path, this simply strips the library name from the input
# path. For a framework, this strips the '.framework/' path and everything
# after it.
#
# Intended for use with bundle packing to define a search path based on a
# (framework) library path.
#
# Usage:
#   get_library_directory(<var> libpath)
#==============================================================================
function(get_library_directory VAR LIBNAME)
  if(LIBNAME MATCHES "^.*\\.framework/.*$")
    # Dealing with a framework library. Go up to the framework directory.
    string(REGEX REPLACE "/[^/]+\\.framework/.*$" "" ${VAR} "${LIBNAME}")
  else(LIBNAME MATCHES "^.*\\.framework/.*$")
    get_filename_component(${VAR} "${LIBNAME}" DIRECTORY)
  endif(LIBNAME MATCHES "^.*\\.framework/.*$")
  set(${VAR} "${${VAR}}" PARENT_SCOPE)
endfunction(get_library_directory)

#==============================================================================
# Converts an unspaced, camel case string into a spaced string. For example,
# this will convert "LibName" to "Lib Name".
#
# As a side effect, any double spaces are converted to single spaces.
#
# Usage:
#   space_camel_case(<var> string)
#==============================================================================
function(space_camel_case VAR STR)
  string(REGEX REPLACE "([A-Z][a-z])" " \\1" ${VAR} "${STR}")
  # We may have inserted spaces before capital letters where there already
  # were spaces. Strip double spaces down to single spaces..
  string(REGEX REPLACE "  " " " ${VAR} "${${VAR}}")
  # We will have inserted a leading space when STR starts with a capital letter.
  # Remove it.
  string(REGEX REPLACE "^ (.*)" "\\1" ${VAR} "${${VAR}}")
  set(${VAR} "${${VAR}}" PARENT_SCOPE)
endfunction(space_camel_case)
