set(TARGET ocurves)
# Use a capital for APPLE bundles.
if(APPLE)
  set(TARGET "OCurves")
endif(APPLE)

set(OCURVES_APP_NAME ${TARGET} CACHE INTERNAL "The main application name.")
set(OCURVES_APP_NAME ${TARGET} PARENT_SCOPE)

if(WIN32)
  set(QT_USE_QTMAIN TRUE)
endif(WIN32)

find_package(BISON QUIET)
find_package(FLEX QUIET)

if(BISON_FOUND AND FLEX_FOUND)
  set(BISON_AND_FLEX YES)
else(BISON_FOUND AND FLEX_FOUND)
  set(BISON_AND_FLEX NO)
endif(BISON_FOUND AND FLEX_FOUND)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# OpenGL, PrintSupport and SVG are not directly required for compiling, but required to run.
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
find_package(Qt5PrintSupport)
find_package(Qt5Concurrent)
find_package(Qt5Svg)
find_package(Qt5Network)
find_package(Qt5SerialPort)
find_package(Qt5Xml)

# Set include directories.
target_config_file(${TARGET} ocurvesconfig.h VAR CONFIG_HEADER)
include_directories(. "${CMAKE_CURRENT_BINARY_DIR}")

include_directories(${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Widgets_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS}
                    ${Qt5OpenGL_INCLUDE_DIRS}
                    ${Qt5Concurrent_INCLUDE_DIRS}
                    ${Qt5PrintSupport_INCLUDE_DIRS}
                    ${Qt5Svg_INCLUDE_DIRS}
                    ${Qt5Network_INCLUDE_DIRS}
                    ${Qt5SerialPort_INCLUDE_DIRS}
                    ${Qt5Xml_INCLUDE_DIRS}
  )

# List source files. Add additional directories here. Defaults to find C++ source files.
# Added to "SOURCES" variable.
list_source_files(SOURCES DIRS . rt ui model)
list_source_files(SOURCES DIRS . icons EXTS png)
list_source_files(QRC_FILES DIRS . EXTS qrc)
if(WIN32)
  list_source_files(SOURCES DIRS . icons EXTS rc ico)
endif(WIN32)
list_source_files(UI_FILES DIRS ui EXTS ui)
# Add documentation headers
list_source_files(DOCHEADERS DIRS doc)
qt5_wrap_ui(QT_SOURCES ${UI_FILES})
qt5_add_resources(QT_RESOURCES ${QRC_FILES})
list(APPEND SOURCES ${QT_SOURCES} ${QT_RESOURCES} ${DOCHEADERS})

#--------------------------------------------------------------------------------
# For Apple set the icns file containing icons
if(APPLE)
  # set how it shows up in the Info.plist file
  set(MACOSX_BUNDLE_ICON_FILE ocurves.icns)
  # set where in the bundle to put the icns file
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/icons/ocurves.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/icons/ocurves.icns)
endif(APPLE)

# Ensure Visual Studio grouping matches that on disk.
group_source_by_dir(${SOURCES} ${UI_FILES} ${QRC_FILES})
group_qt_sources()

set(PACKAGES ${QWT_PACKAGE} plots)

if(ALLOCTRACK_ENABLE)
  list(APPEND PACKAGES alloctrack)
endif(ALLOCTRACK_ENABLE)

if(WIN32)
  set(TARGET_FLAGS WIN32)
endif(WIN32)

include_packages(${PACKAGES})
add_executable(${TARGET} MACOSX_BUNDLE ${TARGET_FLAGS} ${SOURCES} ${UI_FILES})
link_packages(${TARGET} ${PACKAGES})

# Link Qt5 modules
target_link_libraries(${TARGET} Qt5::Widgets Qt5::Gui Qt5::Network Qt5::SerialPort Qt5::Xml Qt5::OpenGL Qt5::Concurrent)

list(APPEND CPACK_PACKAGE_EXECUTABLES "${TARGET};Open Curves")

# Do not use per configuration suffix on MacOS. It messes up Bundles too much.
if(NOT APPLE)
  target_auto_suffix(${TARGET})
endif(NOT APPLE)

add_doxygen_directories(PATH ${SOURCES} ${DOCHEADERS}
                        IMAGE_PATH "icons" "doc/images"
                        EXAMPLE_PATH ".."
                        PARENT_SCOPE)

#-------------------------------------------------------------------------------
# Installation
#-------------------------------------------------------------------------------
install(TARGETS ${TARGET}
  BUNDLE DESTINATION . COMPONENT Runtime
  RUNTIME DESTINATION . COMPONENT Runtime
  #ARCHIVE DESTINATION lib COMPONENT Development
  #LIBRARY DESTINATION lib COMPONENT Development
  )

#--------------------------------------------------------------------------------
# Install needed Qt plugins by copying directories from the qt installation
if(NOT DEFINED OCURVES_PLUGIN_TARGET_DIR)
  set(OCURVES_PLUGIN_TARGET_DIR plugins)
endif(NOT DEFINED OCURVES_PLUGIN_TARGET_DIR)

set(QT_INSTALL_PLUGINS dir MinimalIntegration dir Jpeg)
if(WIN32)
  list(APPEND QT_INSTALL_PLUGINS WindowsPrinterSupport)
endif(WIN32)
install_qt5_plugins(${OCURVES_PLUGIN_TARGET_DIR} ${QT_INSTALL_PLUGINS})

if(TARGET qwt)
target_runtime_dir(QWT_LOC qwt)
else(TARGET qwt)
  get_filename_component(QWT_LOC "${QWT_LIBRARY}" DIRECTORY)
endif(TARGET qwt)

get_target_property(QMAKE_LOC Qt5::qmake LOCATION_Release)
get_target_property(QTCORE_LOC Qt5::Core IMPORTED_LOCATION_RELEASE)
get_filename_component(QT_BINARY_DIR "${QMAKE_LOC}" DIRECTORY)
get_library_directory(QTCORE_LOC "${QTCORE_LOC}")
install_setup_pack(${TARGET} QTCONF BUNDLE COMPONENT Runtime
  FIXDIRS "${CMAKE_INSTALL_PREFIX}/${OCURVES_PLUGIN_TARGET_DIR}"
  SEARCHDIRS "${QT_BINARY_DIR}" "${QWT_LOC}" "${QTCORE_LOC}")
