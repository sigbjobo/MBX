#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "MBX::mbx" for configuration "Debug"
set_property(TARGET MBX::mbx APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(MBX::mbx PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libmbx.so"
  IMPORTED_SONAME_DEBUG "libmbx.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS MBX::mbx )
list(APPEND _IMPORT_CHECK_FILES_FOR_MBX::mbx "${_IMPORT_PREFIX}/lib/libmbx.so" )

# Import target "MBX::mbxlib" for configuration "Debug"
set_property(TARGET MBX::mbxlib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(MBX::mbxlib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/static/libmbxlib.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS MBX::mbxlib )
list(APPEND _IMPORT_CHECK_FILES_FOR_MBX::mbxlib "${_IMPORT_PREFIX}/lib/static/libmbxlib.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
