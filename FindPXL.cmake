# - Try to find PXL 
# Once done, this will define
#
#  PXL_FOUND - system has PXL 
#  PXL_INCLUDE_DIRS - the PXL include directories
#  PXL_LIBRARIES_DIR - link these to use PXL 
#  PXL_PLUGIN_INSTALL_PATH - path to the default plugins (e.g.
#														 $HOME/.pxl-3.0/plugins)
# Note that this only configures the pxl-core system to add a pxl
# plugin, use the PXL_ADD_PLUGIN(name), where name is e.g. pxl-astro


# Use pkg-config to get hints about paths
FIND_PACKAGE(PkgConfig)

find_program(PXLRUN_PATH pxlrun)

if (PKG_CONFIG_FOUND)
    message(STATUS "using pkg config to locate pxl installation")
	pkg_check_modules(pxl_PKGCONF pxl-core)
	if (NOT pxl_PKGCONF_FOUND)
	    message(STATUS "Adjust PKG_CONFIG_PATH in your shell to include the pxl pkg config package")
	endif(NOT pxl_PKGCONF_FOUND)
endif(PKG_CONFIG_FOUND)

SET(PXL_FOUND FALSE)

if (PXLRUN_PATH)
    message(STATUS "pxlrun command found under: ${PXLRUN_PATH}")
	SET(PXL_FOUND TRUE)
	execute_process (COMMAND ${PXLRUN_PATH} --getUserPluginPath
		OUTPUT_VARIABLE PXL_PLUGIN_INSTALL_PATH
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	MESSAGE(STATUS User\ plugin\ path\ modules\ to \ ${PXL_PLUGIN_INSTALL_PATH}  )

	execute_process (COMMAND ${PXLRUN_PATH} --incdir
		OUTPUT_VARIABLE PXL_INCLUDE_DIRS 
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	execute_process (COMMAND ${PXLRUN_PATH} --libdir
		OUTPUT_VARIABLE PXL_LIBRARIES_DIR
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	
  execute_process (COMMAND ${PXLRUN_PATH} --getDataPath
    OUTPUT_VARIABLE PXL_DATA_DIR
		OUTPUT_STRIP_TRAILING_WHITESPACE)
endif(PXLRUN_PATH)


MACRO(ADD_PXL_PLUGIN name)
	MESSAGE(STATUS Adding\ PXL\ plugin \ ${name})
  find_library(lib_${name}_library
    NAMES ${name}
		HINTS ${pxl_PKGCONF_LIBRARY_DIRS} ${PXL_LIBRARIES_DIR}
    )
  LIST(APPEND PXL_LIBRARIES ${lib_${name}_library}) 
ENDMACRO(ADD_PXL_PLUGIN name)

#adds a new module to cmake
MACRO(ADD_PXL_MODULE PXLMODULENAME CPPFILE)

	add_library(
	    ${PXLMODULENAME} MODULE
		${CPPFILE}
	)

	target_link_libraries(
	    ${PXLMODULENAME}
		${PXL_LIBRARIES}
	)
	
	INSTALL(
	    TARGETS 
	    ${PXLMODULENAME} 
	    LIBRARY DESTINATION ${PXL_PLUGIN_INSTALL_PATH}
    )


ENDMACRO(ADD_PXL_MODULE name)





