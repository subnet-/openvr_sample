############################
### Blank OpenVR Project ###
###    use at own risk   ###
############################
cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
cmake_policy(SET CMP0017 NEW)
cmake_policy(SET CMP0020 NEW)


########################
### Application Name ###
########################
set(PROJECTNAME "testapp")

#########################
### Compiler Settings ###
#########################
project("_${PROJECTNAME} OpenVR_" C CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11") # -Wall -pedantic

##########################
### Required libraries ###
##########################
find_package(Qt5OpenGL 5.6.1 REQUIRED)
find_package(OpenVR REQUIRED)

#########################
### process .ui files ###
#########################
qt5_wrap_ui(ui_mainwindow.h gui/mainwindow.ui)

##############################
### process resource files ###
##############################
#qt5_add_resources (ResourceSources shader.qrc)
#qt5_add_resources (ResourceSources images.qrc)
# use this in the future if you want to compile
# the shader and textures inside your program

###########################
### The utility library ###
###########################
add_subdirectory(glbase)

#############################
### Automatic file adding ###
#############################
file(GLOB FILES gui/*.* geometry/*.* shader/*.* vr/*.*)

###################
### The program ###
###################
add_executable(${PROJECTNAME} main.cpp ${ResourceSources} ${FILES})
include_directories(${CMAKE_SOURCE_DIR}/glbase ${OPENGL_INCLUDE_DIR} ${OPENVR_HEADERS_DIR})

########################################
### Build with correct opengl lib    ###
### (this could fail on your system) ###
########################################
if(WIN32 OR CYGWIN)
        target_link_libraries(${PROJECTNAME} opengl32 libglbase Qt5::OpenGL ${OPENGL_gl_LIBRARY} ${OPENVR_LIBRARY})
else()
        target_link_libraries(${PROJECTNAME} GL libglbase Qt5::OpenGL ${OPENGL_gl_LIBRARY} ${OPENVR_LIBRARY})
endif()

######################
### Build settings ###
######################
set_target_properties(${PROJECTNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
file(COPY ${OPENVR_BINARY} DESTINATION "${CMAKE_BINARY_DIR}/bin")