# Copyright (C) 2016
# Computer Graphics Group, University of Siegen
# Written by Martin Lambers <martin.lambers@uni-siegen.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

FIND_PATH(OPENVR_DIR NAMES headers/openvr.h)

FIND_PATH(OPENVR_HEADERS_DIR NAMES openvr.h HINTS ${OPENVR_DIR}/headers)
if(WIN32)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		FIND_LIBRARY(OPENVR_LIBRARY NAMES openvr_api HINTS ${OPENVR_DIR}/lib/win64)
		FIND_LIBRARY(OPENVR_BINARY NAMES openvr_api HINTS ${OPENVR_DIR}/bin/win64)
	else()
		FIND_LIBRARY(OPENVR_LIBRARY NAMES openvr_api HINTS ${OPENVR_DIR}/lib/win32)
		FIND_LIBRARY(OPENVR_BINARY NAMES openvr_api HINTS ${OPENVR_DIR}/bin/win32)
	endif()
else()
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		FIND_LIBRARY(OPENVR_LIBRARY NAMES libopenvr_api HINTS ${OPENVR_DIR}/lib/linux64)
		FIND_LIBRARY(OPENVR_BINARY NAMES libopenvr_api HINTS ${OPENVR_DIR}/bin/linux64)
	else()
		FIND_LIBRARY(OPENVR_LIBRARY NAMES libopenvr_api HINTS ${OPENVR_DIR}/lib/linux32)
		FIND_LIBRARY(OPENVR_BINARY NAMES libopenvr_api HINTS ${OPENVR_DIR}/bin/linux32)
	endif()
endif()

MARK_AS_ADVANCED(OPENVR_HEADERS_DIR OPENVR_LIBRARY OPENVR_BINARY)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENVR
    REQUIRED_VARS OPENVR_LIBRARY OPENVR_HEADERS_DIR OPENVR_BINARY
)

IF(NOT OPENVR_FOUND)
	message(FATAL_ERROR "\nCould not find OpenVR!\nPlease set the variable OPENVR_DIR!\n" )
ENDIF()

