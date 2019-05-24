
find_package(Doxygen QUIET)

if(NOT DOXYGEN_FOUND)
  message(STATUS "Doxygen not found: skipping user documentation generation.")
  return()
endif(NOT DOXYGEN_FOUND)

include(doxygen)

# Create a target to build the documentation.
# Here we also setup various documentation variables passed through to the doxyfile configuration.
# Each named argument below describes the Doxygen variable it sets.
doxygen_create(
  DOXYFILE "${CMAKE_CURRENT_LIST_DIR}/doxyfile.in"  # Doxyfile to configure.
  PROJECT "Open Curves User" # PROJECT_NAME
  TARGET userdoc
  VERSION ${OpenCurves_VERSION}   # PROJECT_NUMBER
  OUTPUT_DIR userdoc            # HTML_OUPTUT
  CSS "${CMAKE_CURRENT_LIST_DIR}/doc.css" # HTML_STYLESHEET
  PUBLISHER "CSIRO"             # DOCSET_PUBLISHER_NAME
  PUBLISHER_ID au.csiro         # DOCSET_PUBLISHER_ID
  PROJECT_ID au.csiro.OpenCurves  # DOCSET_BUNDLE_ID, QHP_NAMESPACE, ECLIPSE_DOC_ID
  PATHS                         # INPUT (RECURSIVE is on)
    .
  # EXCLUDE_PATHS                 # EXCLUDE
  #   alib/private
  # Where to find source code examples.
  EXAMPLE_PATHS                   # EXAMPLE_PATH
    .
  # Where to find images.
  IMAGE_PATHS                   # IMAGE_PATH
    images
    ../ocurves/icons
)

# Setup installation of the generated documentation: source, destination.
doxygen_install(${CMAKE_CURRENT_BINARY_DIR}/userdoc OpenCurves)
