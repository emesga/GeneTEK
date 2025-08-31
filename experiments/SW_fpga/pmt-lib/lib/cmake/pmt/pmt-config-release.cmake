#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "pmt" for configuration "Release"
set_property(TARGET pmt APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(pmt PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libpmt.so"
  IMPORTED_SONAME_RELEASE "libpmt.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS pmt )
list(APPEND _IMPORT_CHECK_FILES_FOR_pmt "${_IMPORT_PREFIX}/lib/libpmt.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
