# This script defines the various installation directories to use for the project.
# This is set up to handle installation to a MacOS bundle.

macro(extract_version VAR PREPROCESSOR_VAR CONTENT)
  # We have to extract the line, then go a replace on just that line to extract
  # a single piece of information.
  set(REGEX_STR "#define[ \t]+${PREPROCESSOR_VAR}[ \t]+([^\n]*)\n")
  string(REGEX MATCH "${REGEX_STR}" ${VAR} "${CONTENT}")
  string(REGEX REPLACE "${REGEX_STR}" "\\1" ${VAR} "${${VAR}}")
endmacro(extract_version)

macro(read_version)
  file(READ ocurves/ocurvesver.h OCURVES_VER_FILE)

  extract_version(OCURVES_MAJOR_VER OCURVES_MAJOR_VER "${OCURVES_VER_FILE}")
  extract_version(OCURVES_MINOR_VER OCURVES_MINOR_VER "${OCURVES_VER_FILE}")
  extract_version(OCURVES_PATCH_VER OCURVES_PATCH_VER "${OCURVES_VER_FILE}")
  extract_version(OCURVES_BUILD_NUMBER OCURVES_BUILD_NUMBER "${OCURVES_VER_FILE}")

  set(OCURVES_VERSION "${OCURVES_MAJOR_VER}.${OCURVES_MINOR_VER}.${OCURVES_PATCH_VER}")

  unset(OCURVES_VER_FILE)
endmacro(read_version)

macro(set_install_path VAR WIN_PATH BUNDLE_PATH LINUX_PATH)
  if(APPLE AND DEFINED OCURVES_BUNDLE_NAME)
    set(${VAR} "${OCURVES_BUNDLE_NAME}.app/${BUNDLE_PATH}")
  elseif(UNIX)
    set(${VAR} "${LINUX_PATH}")
  else()  # WINDOWS
    set(${VAR} "${WIN_PATH}")
  endif(APPLE AND DEFINED OCURVES_BUNDLE_NAME)

  #message("${VAR}: ${${VAR}}")
endmacro(set_install_path)

read_version()
set(DOXYGEN_PROJECT_VERSION "${OCURVES_VERSION}")

space_camel_case(PROJECT_DISPLAY_NAME "${CMAKE_PROJECT_NAME}")

if(APPLE)
  set(OCURVES_BUNDLE_NAME "OCurves")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_DISPLAY_NAME}")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${OCURVES_VERSION}")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${OCURVES_VERSION} Build ${OCURVES_BUILD_NUMBER}")
  set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_DISPLAY_NAME}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${OCURVES_VERSION}")
  set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2015 CSIRO")
endif(APPLE)

set_install_path(OCURVES_BINARY_TARGET_DIR . Contents/MacOS bin)
set_install_path(OCURVES_PLUGIN_TARGET_DIR plugins Contents/MacOS/plugins bin/plugins)
set_install_path(OCURVES_DOC_TARGET_DIR . Contents/Resources .)
set_install_path(OCURVES_SUPPORT_TARGET_DIR . Support .)

# Initialise various CPACK properties
set(CPACK_PACKAGE_VENDOR "${PROJECT_DISPLAY_NAME}")
set(CPACK_PACKAGE_VERSION_MAJOR ${OCURVES_MAJOR_VER})
set(CPACK_PACKAGE_VERSION_MINOR ${OCURVES_MINOR_VER})
set(CPACK_PACKAGE_VERSION_PATCH ${OCURVES_PATCH_VER})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_DISPLAY_NAME})
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_LIST_DIR}/ocurves/icons/ocurves.png")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_LIST_DIR}/license.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_LIST_DIR}/README.md")

set(CPACK_WIX_UPGRADE_GUID "D41E2B26-449A-4E3D-A7E6-75261984BA4E")
set(CPACK_WIX_PROGRAM_MENU_FOLDER "${PROJECT_DISPLAY_NAME}")
set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_LIST_DIR}/ocurves/icons/ocurves.ico")
set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_LIST_DIR}/wix/ocurves-banner.bmp")
set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_LIST_DIR}/wix/ocurves-ui.bmp")
