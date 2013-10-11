CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (AddParticles)
ADD_DEFINITIONS(-std=c++0x)

# Make sure FindPXL.cmake is found.
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

# find PXL
FIND_PACKAGE(PXL)

# make sure the pxl modules code is linked
ADD_PXL_PLUGIN(pxl-modules)
LINK_DIRECTORIES(${PXL_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${PXL_INCLUDE_DIRS})

# set the name of the plugin
SET(PXL_MODULE_NAME AddParticles)

# add the plugin the list of shared libraries to be build
ADD_LIBRARY(${PXL_MODULE_NAME} MODULE AddParticles.cpp)

# add the pxl libraries as dependencies
TARGET_LINK_LIBRARIES (${PXL_MODULE_NAME} ${PXL_LIBRARIES})

# Install the module in the user home directory
INSTALL(TARGETS ${PXL_MODULE_NAME} LIBRARY DESTINATION ${PXL_PLUGIN_INSTALL_PATH})
