#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mozjpeg::jpeg" for configuration "Debug"
set_property(TARGET mozjpeg::jpeg APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mozjpeg::jpeg PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/jpeg.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/jpeg62.dll"
  )

list(APPEND _cmake_import_check_targets mozjpeg::jpeg )
list(APPEND _cmake_import_check_files_for_mozjpeg::jpeg "${_IMPORT_PREFIX}/lib/jpeg.lib" "${_IMPORT_PREFIX}/bin/jpeg62.dll" )

# Import target "mozjpeg::turbojpeg" for configuration "Debug"
set_property(TARGET mozjpeg::turbojpeg APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mozjpeg::turbojpeg PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/turbojpeg.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/turbojpeg.dll"
  )

list(APPEND _cmake_import_check_targets mozjpeg::turbojpeg )
list(APPEND _cmake_import_check_files_for_mozjpeg::turbojpeg "${_IMPORT_PREFIX}/lib/turbojpeg.lib" "${_IMPORT_PREFIX}/bin/turbojpeg.dll" )

# Import target "mozjpeg::turbojpeg-static" for configuration "Debug"
set_property(TARGET mozjpeg::turbojpeg-static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mozjpeg::turbojpeg-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/turbojpeg-static.lib"
  )

list(APPEND _cmake_import_check_targets mozjpeg::turbojpeg-static )
list(APPEND _cmake_import_check_files_for_mozjpeg::turbojpeg-static "${_IMPORT_PREFIX}/lib/turbojpeg-static.lib" )

# Import target "mozjpeg::jpeg-static" for configuration "Debug"
set_property(TARGET mozjpeg::jpeg-static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mozjpeg::jpeg-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/jpeg-static.lib"
  )

list(APPEND _cmake_import_check_targets mozjpeg::jpeg-static )
list(APPEND _cmake_import_check_files_for_mozjpeg::jpeg-static "${_IMPORT_PREFIX}/lib/jpeg-static.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
