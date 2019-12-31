#.rst:
# FindTexturePacker
# -----------------
# Finds the TexturePacker
#
# If WITH_TEXTUREPACKER is defined and points to a directory,
# this path will be used to search for the Texturepacker binary
#
#
# This will define the following (imported) targets::
#
#   TexturePacker::TexturePacker   - The TexturePacker executable

include(cmake/scripts/common/BuildNative.cmake)

if(NOT TARGET TexturePacker::TexturePacker)
  if(KODI_DEPENDSBUILD)
    add_executable(TexturePacker::TexturePacker IMPORTED GLOBAL)
    set_target_properties(TexturePacker::TexturePacker PROPERTIES
                                                       IMPORTED_LOCATION "${NATIVEPREFIX}/bin/TexturePacker")
  else()
    if(CMAKE_HOST_WIN32 AND NOT WITH_TEXTUREPACKER)
      build_native_tool(NAME TexturePacker
                        SOURCE_DIR ${CMAKE_SOURCE_DIR}/tools/depends/native/TexturePacker
                        BUILD_DIR ${CMAKE_BINARY_DIR}/TexturePacker-build
                        EXTRA_ARGS
                          -A Win32
                          -DCMAKE_PREFIX_PATH=${CMAKE_SOURCE_DIR}/project/BuildDependencies/native
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set(WITH_TEXTUREPACKER ${TEXTUREPACKER_DIR})
    endif()
    if(WITH_TEXTUREPACKER)
      get_filename_component(_tppath ${WITH_TEXTUREPACKER} ABSOLUTE)
      find_program(TEXTUREPACKER_EXECUTABLE TexturePacker PATHS ${_tppath})

      include(FindPackageHandleStandardArgs)
      find_package_handle_standard_args(TexturePacker DEFAULT_MSG TEXTUREPACKER_EXECUTABLE)
      if(TEXTUREPACKER_FOUND)
        add_executable(TexturePacker::TexturePacker IMPORTED GLOBAL)
        set_target_properties(TexturePacker::TexturePacker PROPERTIES
                                                           IMPORTED_LOCATION "${TEXTUREPACKER_EXECUTABLE}")
      endif()
      mark_as_advanced(TEXTUREPACKER)
    endif()
  endif()
endif()
