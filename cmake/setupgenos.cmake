#==============================================================================
# Generic OS extensions.
#==============================================================================

include(CMakeParseArguments)

if(NOT DEFINED SHARED_LIB_EXPORT)
  set(SHARED_LIB_EXPORT __attribute__((visibility(\"default\"))))
endif(NOT DEFINED SHARED_LIB_EXPORT)

if(NOT DEFINED SHARED_LIB_IMPORT)
  set(SHARED_LIB_IMPORT __attribute__((visibility(\"default\"))))
endif(NOT DEFINED SHARED_LIB_IMPORT)

if(NOT SHARED_LIB_HIDDEN)
  set(SHARED_LIB_HIDDEN __attribute__((visibility(\"hidden\"))))
endif(NOT SHARED_LIB_HIDDEN)

if(NOT COMMAND enable_release_debug_info)
  function(enable_release_debug_info)
  endfunction(enable_release_debug_info)
endif(NOT COMMAND enable_release_debug_info)
