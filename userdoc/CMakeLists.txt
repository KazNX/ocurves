
find_package(Doxygen QUIET)

set(DOXYGEN_PROJECT "User Documentation")
file(GLOB HEADERS *.h *.css)
set(DOXYGEN_INPUT_LIST "${CMAKE_CURRENT_LIST_DIR}")
set(DOXYGEN_IMAGE_PATH "${CMAKE_CURRENT_LIST_DIR}/images ${CMAKE_CURRENT_LIST_DIR}/../ocurves/icons")
set(DOXYGEN_EXAMPLE_PATH "${CMAKE_CURRENT_LIST_DIR}")
set(DOXYGEN_CSS "${CMAKE_CURRENT_LIST_DIR}/doc.css")
set(DOXYGEN_HTML_OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/userdoc")
configure_file(doxyfile.in "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

if(DOXYGEN_FOUND)
  add_custom_target(userdoc
                    "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" ALL
                    DEPENDS
                      doxyfile.in
                      "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
                      SOURCES ${HEADERS} doxyfile.in
                    )
  install(DIRECTORY "${DOXYGEN_HTML_OUTPUT}" DESTINATION "${OCURVES_DOC_TARGET_DIR}" COMPONENT Runtime)
else(DOXYGEN_FOUND)
  add_custom_target(userdoc
                      SOURCES ${HEADERS} doxyfile.in
                    )
endif(DOXYGEN_FOUND)
