#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "png_shared" for configuration "MinSizeRel"
set_property(TARGET png_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(png_shared PROPERTIES
  IMPORTED_IMPLIB_MINSIZEREL "${_IMPORT_PREFIX}/lib/libpng16.lib"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/bin/libpng16.dll"
  )

list(APPEND _cmake_import_check_targets png_shared )
list(APPEND _cmake_import_check_files_for_png_shared "${_IMPORT_PREFIX}/lib/libpng16.lib" "${_IMPORT_PREFIX}/bin/libpng16.dll" )

# Import target "png_static" for configuration "MinSizeRel"
set_property(TARGET png_static APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(png_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "C"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/libpng16_static.lib"
  )

list(APPEND _cmake_import_check_targets png_static )
list(APPEND _cmake_import_check_files_for_png_static "${_IMPORT_PREFIX}/lib/libpng16_static.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
