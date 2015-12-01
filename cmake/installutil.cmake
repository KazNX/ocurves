#==============================================================================
# Contains various export and installation utility functions.
#==============================================================================
include(CMakeParseArguments)

#===============================================================================
#===============================================================================
function(install_qt5_plugins PLUGIN_DEST_DIR)
  CMAKE_PARSE_ARGUMENTS(IQP "" "" "" ${ARGN})

  # Use the QMinimalIntegrationPlugin to find the plugins directory.
  # Use qmake to find the binary directory.
  get_target_property(QMINIMAL_LOC Qt5::QMinimalIntegrationPlugin LOCATION_Release)
  get_filename_component(QT_PLUGINS_DIR "${QMINIMAL_LOC}" DIRECTORY)
  get_filename_component(QT_PLUGINS_DIR "${QT_PLUGINS_DIR}" DIRECTORY)

  if(NOT QT_PLUGINS_DIR)
    # Cannot find, or no need to install plugins.
    # This trips for Linux, where Qt will generally be installed in system folders.
    return()
  endif(NOT QT_PLUGINS_DIR)

  if(UNIX AND NOT APPLE)
    return()
  endif(UNIX AND NOT APPLE)

  if(WIN32)
    set(DEBUG_REGEX "[^\\.]+d\\..+$")
  else(WIN32)
    set(DEBUG_REGEX "[^\\.]+_debug\\..+$")
  endif(WIN32)

  foreach(PLUGINDIR ${IQP_UNPARSED_ARGUMENTS})
    if(CMAKE_CONFIGURATION_TYPES)
      # Restrict installation in a multi-configuration setup.
      install(DIRECTORY "${QT_PLUGINS_DIR}/${PLUGINDIR}" DESTINATION ${PLUGIN_DEST_DIR} COMPONENT Runtime
              CONFIGURATIONS Release RelWithDebInfo MinSizeRel
              REGEX "${DEBUG_REGEX}" EXCLUDE)
      install(DIRECTORY "${QT_PLUGINS_DIR}/${PLUGINDIR}" DESTINATION ${PLUGIN_DEST_DIR} COMPONENT Runtime
              CONFIGURATIONS Debug
              REGEX "${DEBUG_REGEX}")
    else(CMAKE_CONFIGURATION_TYPES)
      # Single configuration. Only consider whether to use debug libraries or not.
      if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
      install(DIRECTORY "${QT_PLUGINS_DIR}/${PLUGINDIR}" DESTINATION ${PLUGIN_DEST_DIR} COMPONENT Runtime
              REGEX "${DEBUG_REGEX}")
      else(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
        install(DIRECTORY "${QT_PLUGINS_DIR}/${PLUGINDIR}" DESTINATION ${PLUGIN_DEST_DIR} COMPONENT Runtime
                REGEX "${DEBUG_REGEX}" EXCLUDE)
      endif(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    endif(CMAKE_CONFIGURATION_TYPES)
  endforeach(PLUGINDIR)
endfunction(install_qt5_plugins)

#===============================================================================
# Get the RUNTIME_OUTPUT_DIRECTORY for a target, catering for multi-configuration
# and single-configuration builds
#===============================================================================
function(target_runtime_dir VAR TARGET)
  set(DIR_LIST)
  set(${VAR} VAR-NOTFOUND PARENT_SCOPE)
  get_target_property(DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY)
  if(DIR)
    list(APPEND DIR_LIST "${DIR}")
  endif()
  get_target_property(DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY_DEBUG)
  if(DIR)
    list(APPEND DIR_LIST "${DIR}")
  endif()
  get_target_property(DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL)
  if(DIR)
    list(APPEND DIR_LIST "${DIR}")
  endif()
  get_target_property(DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY_RELEASE)
  if(DIR)
    list(APPEND DIR_LIST "${DIR}")
  endif()
  get_target_property(DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO)
  if(DIR)
    list(APPEND DIR_LIST "${DIR}")
  endif()
  set(${VAR} "${DIR_LIST}" PARENT_SCOPE)
endfunction(target_runtime_dir)

#===============================================================================
# BUNDLE - treat as a bundle installation
# FIXDIRS - additional fixup directories
# SEARCHDIRS - additional search directories
# COMPONENT - install component. Default to Runtime
#===============================================================================
function(install_setup_pack TARGET)
  CMAKE_PARSE_ARGUMENTS(ISP "QTCONF;BUNDLE" "COMPONENT" "FIXDIRS;SEARCHDIRS" ${ARGN})

  if(NOT ISP_COMPONENT)
    set(ISP_COMPONENT Runtime)
  endif(NOT ISP_COMPONENT)

  #----------------------------------------------------------
  # Setup packing variables. Default to pack to bin directory
  set(plugin_conf_dir bin)
  set(qtconf_dest_dir bin)
  set(APP_LIST)
  # Handle multi configuration builds
  set(TARGET_NAME "${TARGET}")
  if(CMAKE_CONFIGURATION_TYPES)
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER ${CONFIG} CONFIGUP)
      get_target_property(TARGET_NAME ${TARGET} OUTPUT_NAME_${CONFIGUP})
      if(NOT TARGET_NAME)
        set(TARGET_NAME ${TARGET})
      endif(NOT TARGET_NAME)
      list(APPEND APP_LIST ${CONFIG} "\${CMAKE_INSTALL_PREFIX}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
    endforeach(CONFIG)
  else(CMAKE_CONFIGURATION_TYPES)
    # Single configuration build
    get_target_property(TARGET_NAME ${TARGET} OUTPUT_NAME)
    if(NOT TARGET_NAME)
      set(TARGET_NAME ${TARGET})
    endif(NOT TARGET_NAME)
    list(APPEND APP_LIST "\${CMAKE_INSTALL_PREFIX}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
  endif(CMAKE_CONFIGURATION_TYPES)
  set(BUNDLE_NAME "${TARGET_NAME}")
  #list(REMOVE_DUPLICATES APP_LIST)
  #message("APP_LIST: ${APP_LIST}")

  if(APPLE AND ISP_BUNDLE)
    #message("BUNDLE-BUNDLE-BUNDLE-BUNDLE-BUNDLE-BUNDLE")
    # Setup APPLE bundle
    set(qtconf_dest_dir ${BUNDLE_NAME}.app/Contents/Resources)
    # Relative path in bundle to find plugins directory.
    set(plugin_conf_dir ../Contents/MacOS)
    set(APP_LIST "\${CMAKE_INSTALL_PREFIX}/${BUNDLE_NAME}.app")
  endif(APPLE AND ISP_BUNDLE)
  if(WIN32)
    # For windows don't use a bin directory.
    set(plugin_conf_dir .)
    SET(qtconf_dest_dir .)
    # message("APP_LIST: ${APP_LIST}")
  endif(WIN32)

  #----------------------------------------------------------
  # install a qt.conf file
  # this inserts some cmake code into the install script to write the file
  # Ensure the plugins directory is correctly specified.
  if(ISP_QTCONF)
    INSTALL(CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\"
\"[Paths]
Prefix = ${plugin_conf_dir}
\")
          " COMPONENT ${ISP_COMPONENT})
  endif(ISP_QTCONF)

  #----------------------------------------------------------
  # Use BundleUtilities to get all other dependencies for the application to work.
  # It takes a bundle or executable along with possible plugins and inspects it
  # for dependencies.  If they are not system dependencies, they are copied.

  # directories to look for dependencies
  SET(DIRS ${QT_LIBRARY_DIRS})

  # Now the work of copying dependencies into the bundle/package
  # The quotes are escaped and variables to use at install time have their $ escaped
  # An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
  # Note that the image plugins depend on QtSvg and QtXml, and it got those copied
  # over.
  if(ISP_BUNDLE OR WIN32)
    INSTALL(CODE "
        set(APPS_LIST)
        set(USE_NEXT_APP NO)
        set(APP_LIST ${APP_LIST})
        list(LENGTH APP_LIST APPCOUNT)
        if(APPCOUNT GREATER 1)
          foreach(APPITEM \${APP_LIST})
            if(USE_NEXT_APP)
              set(APP \"\${APPITEM}\")
              set(USE_NEXT_APP NO)
            elseif(\"\${CMAKE_INSTALL_CONFIG_NAME}\" MATCHES \"\${APPITEM}\")
              set(USE_NEXT_APP YES)
            else()
              set(USE_NEXT_APP NO)
            endif()
            if(EXISTS \"\${APP}\")
              list(APPEND APPS_LIST \"\${APP}\")
            endif()
          endforeach(APPITEM)
        else(APPCOUNT GREATER 1)
          set(APP \"\${APP_LIST}\")
        endif(APPCOUNT GREATER 1)
        foreach(DIR ${ISP_FIXDIRS})
          file(GLOB_RECURSE FIXLIBS \"\${DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
        endforeach(DIR)
        include(BundleUtilities)
        #message(\"======================================================================\")
        #message(\"APP: \${APP}\")
        #message(\"FIXLIBS: \${FIXLIBS}\")
        #message(\"======================================================================\")
        fixup_bundle(\"\${APP}\" \"\${FIXLIBS}\" \"${ISP_SEARCHDIRS}\")
        " COMPONENT ${ISP_COMPONENT})
  endif(ISP_BUNDLE OR WIN32)

  # To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
  # where CPackConfig.cmake is created by including CPack
  # And then there's ways to customize this as well
  if(APPLE)
    set(CPACK_BINARY_DRAGNDROP ON)
  endif(APPLE)
  include(CPack)
endfunction(install_setup_pack)
