#==============================================================================
# Contains various export and installation utility functions.
#==============================================================================
include(CMakeParseArguments)


#===============================================================================
# A helper for install_qt5_plugins, setting up the installation of PLUGIN for
# the specified CONFIG.
#
# 
#===============================================================================
function(_install_qt5_plugin PLUGIN CONFIG QTCONFIG DIRECTORY_INSTALL)
  get_target_property(QPLUGIN_LOC Qt5::Q${PLUGIN}Plugin LOCATION_${QTCONFIG})
  #message(":::::::::::::::::::::::::::::::::::::::::::::")
  #message("Qt5::Q${PLUGIN}Plugin @ ${QPLUGIN_LOC}")
  get_filename_component(QPLUGIN_DIR "${QPLUGIN_LOC}" DIRECTORY)
  get_filename_component(QPLUGIN_SUBDIR "${QPLUGIN_DIR}" NAME)
  if(NOT DIRECTORY_INSTALL)
    install(FILES "${QPLUGIN_LOC}" DESTINATION "${PLUGIN_DEST_DIR}/${QPLUGIN_SUBDIR}" COMPONENT Runtime CONFIGURATIONS ${CONFIG})
  else(NOT DIRECTORY_INSTALL)
    # Here we will create a regular expression to match shared library files either with
    # or without the expected debug library suffix. On Windows it's 'd', on MacOS it's "_debug"
    # and probably so on Linux to.
    unset(DEBUG_SUFFIX)
    if(WIN32)
      set(DEBUG_SUFFIX d)
    else(WIN32)
      set(DEBUG_SUFFIX _debug)
    endif(WIN32)

    # For non-debug change exclude the DEBUG_SUFFIX.
    if(NOT CONFIG STREQUAL "Debug")
      set(EXCLUDE EXCLUDE)
    endif(NOT CONFIG STREQUAL "Debug")

    # Set up the expected file extension. We strip any leading '.' character
    # from CMAKE_SHARED_LIBRARY_SUFFIX because it's easier to create the
    # regular expression that way.
    set(LIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    if(LIB_SUFFIX)
      string(SUBSTRING ${LIB_SUFFIX} 0 1 CHAR)
      if(CHAR STREQUAL ".")
        string(LENGTH ${LIB_SUFFIX} LEN)
        math(EXPR LEN "${LEN} - 1")
        string(SUBSTRING ${LIB_SUFFIX} 1 ${LEN} LIB_SUFFIX)
      endif(CHAR STREQUAL ".")
    endif(LIB_SUFFIX)

    # Specify the matching expression. We expect:
    #   <something>[suffix][version].{dll|so|dylib}
    # where suffix may match or exclude _debug or d name suffixing.
    set(LIB_REGEX "[^\\.]+${DEBUG_SUFFIX}\\.[0-9\\.]*${LIB_SUFFIX}")

    install(DIRECTORY "${QPLUGIN_DIR}" DESTINATION "${PLUGIN_DEST_DIR}"
            COMPONENT Runtime
            CONFIGURATIONS ${CONFIG}
            REGEX "${LIB_REGEX}" ${EXCLUDE}
            PATTERN "*.lib" EXCLUDE
            PATTERN "*.exp" EXCLUDE
            PATTERN "*.ilk" EXCLUDE
            PATTERN "*.pdb" EXCLUDE
            )
  endif(NOT DIRECTORY_INSTALL)
endfunction(_install_qt5_plugin)

#===============================================================================
# install_qt5_plugins(PLUGIN_DEST_DIR plugin1 plugin2 ... dir pluginX)
#
# Sets up installation of a list of Qt plugins. In order to function, end
# requested plugin must be defined as a valid target. This occurs normally when
# using a find_package(Qt5<Module>) directive. The plugins for that module will
# be defined. Some plugins may be omitted, in which case the entire plugin
# directory may be marshalled based on a plugin which is defined as a target.
# Adding 'dir' before the plugin name triggers this behaviour.
#
# For example, the Qt5::GUI plugins may be marshalled via the following
# commands:
#
#       find_package(Qt5::GUI)
#       install_qt5_plugins("plugins" dir MinimalIntegration)
#
# This uses the directory of Qt5::QMinimalIntegrationPlugin as the directory
# to install.
#===============================================================================
function(install_qt5_plugins PLUGIN_DEST_DIR)
  CMAKE_PARSE_ARGUMENTS(IQP "" "" "" ${ARGN})

  set(DIRECTORY_INSTALL NO)
  foreach(PLUGIN ${IQP_UNPARSED_ARGUMENTS})
    if("${PLUGIN}" STREQUAL "dir")
      set(DIRECTORY_INSTALL YES)
    else("${PLUGIN}" STREQUAL "dir")
      if(CMAKE_CONFIGURATION_TYPES)
        foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
          # Use Qt Debug only with a debug build. Release for all others.
          if(CONFIG_TYPE STREQUAL "Debug")
            set(CONFIG "Debug")
          else(CONFIG_TYPE STREQUAL "Debug")
            set(CONFIG "Release")
          endif(CONFIG_TYPE STREQUAL "Debug")
          _install_qt5_plugin(${PLUGIN} ${CONFIG_TYPE} ${CONFIG} ${DIRECTORY_INSTALL})
        endforeach(CONFIG_TYPE)
      else(CMAKE_CONFIGURATION_TYPES)
        if(CMAKE_BUILD_TYPE)
          set(CONFIG ${CMAKE_BUILD_TYPE})
        else(CMAKE_BUILD_TYPE)
          set(CONFIG Release)
        endif(CMAKE_BUILD_TYPE)

        _install_qt5_plugin(${PLUGIN} ${CONFIG} ${CONFIG} ${DIRECTORY_INSTALL})
      endif(CMAKE_CONFIGURATION_TYPES)

      set(DIRECTORY_INSTALL NO)
    endif("${PLUGIN}" STREQUAL "dir")
  endforeach(PLUGIN)
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
# BINDIR - the relative installation directory for the executable/applications.
#     E.g, specify 'bin' if installing to a bin directory under the install path.
# FIXDIRS - additional fixup directories
# SEARCHDIRS - additional search directories
# COMPONENT - install component. Default to Runtime
#===============================================================================
function(install_setup_pack TARGET)
  CMAKE_PARSE_ARGUMENTS(ISP "QTCONF;BUNDLE" "COMPONENT" "BINDIR;FIXDIRS;SEARCHDIRS" ${ARGN})

  if(NOT ISP_COMPONENT)
    set(ISP_COMPONENT Runtime)
  endif(NOT ISP_COMPONENT)

  #----------------------------------------------------------
  # Setup packing variables. Default to pack to bin directory
  if(NOT ISP_BINDIR)
    if(WIN32)
      set(ISP_BINDIR .)
    else(WIN32)
      set(ISP_BINDIR bin)
    endif(WIN32)
  endif(NOT ISP_BINDIR)
  set(plugin_conf_dir .)
  set(qtconf_dest_dir ${ISP_BINDIR})

    set(app_dir_prefix)
  if(ISP_BINDIR AND NOT "${ISP_BINDIR}" STREQUAL ".")
    set(app_dir_prefix "${ISP_BINDIR}/")
  endif(ISP_BINDIR AND NOT "${ISP_BINDIR}" STREQUAL ".")

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
      list(APPEND APP_LIST ${CONFIG} "\${CMAKE_INSTALL_PREFIX}/${app_dir_prefix}${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
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
