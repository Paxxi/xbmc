include(cmake/scripts/common/DownloadDistfile.cmake)
include(cmake/scripts/common/ExtractDistfile.cmake)

function(download_and_extract_depends)
  if(NOT CMAKE_HOST_WIN32)
    return()
  endif()

  set(_daed_BASE_DIR ${CMAKE_SOURCE_DIR}/project/BuildDependencies)
  set(_daed_DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/project/BuildDependencies/downloads)
  set(_daed_PLATFORM_DIR ${CMAKE_VS_PLATFORM_NAME})
  string(TOUPPER ${CMAKE_VS_PLATFORM_NAME} _daed_PLATFORM)

  if(WINDOWS_STORE)
    include(cmake/scripts/windowsstore/Depends.cmake)
    set(_daed_PLATFORM_DIR "win10-${_daed_PLATFORM_DIR}")
  else()
    include(cmake/scripts/windows/Depends.cmake)
  endif()

  _download_and_extract_depends("${DEPENDS_TO_DOWNLOAD_NATIVE}"
                                ${_daed_DOWNLOAD_DIR}
                                ${_daed_BASE_DIR}/native
                                ${_daed_BASE_DIR})
  _download_and_extract_depends("${DEPENDS_TO_DOWNLOAD_${_daed_PLATFORM}}"
                                ${_daed_DOWNLOAD_DIR}
                                ${_daed_BASE_DIR}/${_daed_PLATFORM}
                                ${_daed_BASE_DIR})

endfunction()

function(_download_and_extract_depends DEPENDS DOWNLOAD_DIR OUT_DIR BASE_DIR)
  set(_daed_BASE_URL "http://mirrors.kodi.tv/build-deps/win32")
  foreach(_daed_line ${DEPENDS})
    string(REGEX MATCH "([a-zA-z0-9.-]+):([a-zA-Z0-9]+)" _daed_parts ${_daed_line})
    download_distfile(_daed_downloaded_file
      URLS ${_daed_BASE_URL}/${CMAKE_MATCH_1}
      FILENAME ${CMAKE_MATCH_1}
      SHA512 ${CMAKE_MATCH_2}
      DOWNLOAD_DIR ${DOWNLOAD_DIR}
      NEW_FILE_VAR _daed_is_new_file
    )

    extract_distfile(
      SOURCE_PATH ${OUT_DIR}
      ARCHIVE ${_daed_downloaded_file}
      WORKING_DIRECTORY ${BASE_DIR}
      FORCE ${_daed_is_new_file}
    )
  endforeach()
endfunction()