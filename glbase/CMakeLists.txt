# Copyright (C) 2013, 2014
# Computer Graphics Group, University of Siegen
# Written by Martin Lambers <martin.lambers@uni-siegen.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

# The library
include_directories(${CMAKE_SOURCE_DIR})
add_library(libglbase STATIC
	geomload.hpp geomload.cpp
	glew.c GL/glew.h GL/glxew.h GL/wglew.h
	gltool.hpp gltool.cpp
	ply.h plyfile.cpp
	tiny_obj_loader.h tiny_obj_loader.cc)

set_target_properties(libglbase PROPERTIES OUTPUT_NAME glbase)
if(WIN32 OR CYGWIN)
        target_link_libraries(libglbase opengl32)
else()
		target_link_libraries(libglbase GL)
endif()