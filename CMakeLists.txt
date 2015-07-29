cmake_minimum_required(VERSION 2.8)
PROJECT(libsdo)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)
FIND_PACKAGE(Boost COMPONENTS system filesystem locale REQUIRED)

IF(BISON_FOUND AND FLEX_FOUND)
    FOREACH(Prefix Mdl Voc Vpd Vop)	
	ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${Prefix}Parser.cpp
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sdo/${Prefix}Parser.y
		COMMAND ${BISON_EXECUTABLE} -v
		-d -o  ${CMAKE_CURRENT_BINARY_DIR}/${Prefix}Parser.cpp
		--name-prefix ${Prefix}
                ${CMAKE_CURRENT_SOURCE_DIR}/sdo/${Prefix}Parser.y
		COMMENT "Generating ${Prefix}Parser.cpp"
	)
	ADD_CUSTOM_COMMAND(
        	OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${Prefix}Lexer.cpp
        	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sdo/${Prefix}Lexer.l
                COMMAND ${FLEX_EXECUTABLE}
                -o ${CMAKE_CURRENT_BINARY_DIR}/${Prefix}Lexer.cpp
		-P ${Prefix}
                ${CMAKE_CURRENT_SOURCE_DIR}/sdo/${Prefix}Lexer.l
                COMMENT "Generating ${Prefix}Lexer.cpp"
	)
    ENDFOREACH(Prefix)
ENDIF()

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)


INCLUDE_DIRECTORIES(
	${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/sdo
)

ADD_LIBRARY(sdo STATIC
	sdo/ButcherTableau.cpp
	sdo/ReadFile.cpp
	sdo/RandomUniform.cpp
	sdo/FileStatus.cpp
	sdo/ExpressionGraph.cpp
	${CMAKE_CURRENT_BINARY_DIR}/MdlParser.cpp
	${CMAKE_CURRENT_BINARY_DIR}/MdlLexer.cpp
	${CMAKE_CURRENT_BINARY_DIR}/VpdParser.cpp
	${CMAKE_CURRENT_BINARY_DIR}/VpdLexer.cpp
	${CMAKE_CURRENT_BINARY_DIR}/VocParser.cpp
	${CMAKE_CURRENT_BINARY_DIR}/VocLexer.cpp
	${CMAKE_CURRENT_BINARY_DIR}/VopParser.cpp
	${CMAKE_CURRENT_BINARY_DIR}/VopLexer.cpp
	)

set_target_properties( sdo PROPERTIES COMPILE_FLAGS "-std=c++11 -pedantic-errors -Wall -Wextra -Wno-unused-parameter" )
set( libsdo_LIBRARY sdo )

TARGET_LINK_LIBRARIES(sdo ${Boost_LIBRARIES})
FILE( GLOB header_files "${CMAKE_CURRENT_SOURCE_DIR}/sdo/*.hpp")
INSTALL( FILES ${header_files} DESTINATION include/sdo)
INSTALL( TARGETS sdo EXPORT sdo-targets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


set ( libsdo_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" )


set ( CMAKE_CONFIG_DEST "share/cmake/libsdo" )

include ( CMakePackageConfigHelpers )

configure_package_config_file (                    
        "${CMAKE_CURRENT_SOURCE_DIR}/libsdo-config.cmake.in"
        "${CMAKE_BINARY_DIR}/libsdo-config.cmake"  
        INSTALL_DESTINATION "${CMAKE_CONFIG_DEST}"
        PATH_VARS libsdo_INCLUDE_DIRS )

export ( TARGETS sdo FILE ${CMAKE_BINARY_DIR}/libsdo-targets.cmake )

export ( PACKAGE libsdo )

set ( libsdo_INCLUDE_DIRS "include" )

configure_package_config_file ( 
       "${CMAKE_CURRENT_SOURCE_DIR}/libsdo-config.cmake.in" 
       "${CMAKE_BINARY_DIR}/export/libsdo-config.cmake"
       INSTALL_DESTINATION "${CMAKE_CONFIG_DEST}"
       PATH_VARS libsdo_INCLUDE_DIRS )

install (
      EXPORT sdo-targets 
      DESTINATION ${CMAKE_CONFIG_DEST} FILE "libsdo-targets.cmake" )
INSTALL(
  FILES ${CMAKE_BINARY_DIR}/export/libsdo-config.cmake
  DESTINATION ${CMAKE_CONFIG_DEST}
)
