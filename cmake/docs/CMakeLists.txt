
set(OTHER_DOX
	../exosite-lib/src/exosite_c_libraries.dox
	)

#find_package(PlantUML)
find_program(PLANTUML_EXECUTABLE
	NAME plantuml
	DOC "Creating the UML based images")


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})
if(BUILD_DOCUMENTATION)
	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Doxygen is needed to build the documentation.")
	endif()

	#set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
	set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

	#configure_file(${doxyfile_in} ${doxyfile} @ONLY)

	add_custom_target(doc
		COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM)

	#install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif(BUILD_DOCUMENTATION)

if(PLANTUML_EXECUTABLE)
	add_custom_target(uml_images
		COMMAND ${PLANTUML_EXECUTABLE} v -o "${CMAKE_CURRENT_BINARY_DIR}/html/" ${OTHER_DOX}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating PNGs from inlined UML with PlantUML"
		VERBATIM)

	add_dependencies(doc uml_images)
endif(PLANTUML_EXECUTABLE)

#  vim: set sw=4 ts=4 :
