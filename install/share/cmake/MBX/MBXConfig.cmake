
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was MBXConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set(PN MBX)

get_filename_component(MBX_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(MBX_ORIG_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${MBX_CMAKE_DIR})

# find MBX's FFTW dependency
include(CMakeFindDependencyMacro)
find_dependency(FFTW)

set(CMAKE_MODULE_PATH ${MBX_ORIG_CMAKE_MODULE_PATH})


if(NOT TARGET ${PN}::mbxlib)
    include("${CMAKE_CURRENT_LIST_DIR}/${PN}Targets.cmake")
endif()
