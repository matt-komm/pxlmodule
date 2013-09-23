CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (TTreeFiller)
ADD_DEFINITIONS(-std=c++0x)

# Make sure FindPXL.cmake is found.
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

# find PXL
FIND_PACKAGE(PXL)
FIND_PACKAGE(ROOT)

# make sure the pxl modules code is linked
ADD_PXL_PLUGIN(pxl-modules)
LINK_DIRECTORIES(${PXL_LIBRARY_DIRS} ${ROOT_LIBRARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${PXL_INCLUDE_DIRS} ${ROOT_INCLUDE_DIR})

# set the name of the plugin
SET(PXL_MODULE_NAME TTreeFiller)

# add the plugin the list of shared libraries to be build
ADD_LIBRARY(${PXL_MODULE_NAME} MODULE TTreeFiller.cpp OutputStore.cpp)

# add the pxl libraries as dependencies
TARGET_LINK_LIBRARIES (${PXL_MODULE_NAME} ${PXL_LIBRARIES} ${ROOT_LIBRARIES})

# Install the module in the user home directory
INSTALL(TARGETS ${PXL_MODULE_NAME} LIBRARY DESTINATION ${PXL_PLUGIN_INSTALL_PATH})
