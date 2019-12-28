## # download_distfile
##
## Download and cache a file needed for this port.
##
## This helper should always be used instead of CMake's built-in `file(DOWNLOAD)` command.
##
## ## Usage
## ```cmake
## download_distfile(
##     <OUT_VARIABLE>
##     URLS <http://mainUrl> <http://mirror1>...
##     FILENAME <output.zip>
##     SHA512 <5981de...>
## )
## ```
## ## Parameters
## ### OUT_VARIABLE
## This variable will be set to the full path to the downloaded file. This can then immediately be passed in to `extract_distfile` for sources.
##
## ### URLS
## A list of URLs to be consulted. They will be tried in order until one of the downloaded files successfully matches the SHA512 given.
##
## ### FILENAME
## The local name for the file. Files are shared between ports, so the file may need to be renamed to make it clearly attributed to this port and avoid conflicts.
##
## ### SHA512
## The expected hash for the file.
##
## ### DOWNLOAD_DIR
## The directory to cache the downloaded file
##
## If this doesn't match the downloaded version, the build will be terminated with a message describing the mismatch.
##
## ### SKIP_SHA512
## Skip SHA512 hash check for file.
##
## This switch is only valid when building with the `--head` command line flag.
##
## ### HEADERS
## A list of headers to append to the download request. This can be used for authentication during a download.
##
## Headers should be specified as "<header-name>: <header-value>".
function(download_distfile VAR)
    set(options SKIP_SHA512)
    set(oneValueArgs FILENAME SHA512 DOWNLOAD_DIR NEW_FILE_VAR)
    set(multipleValuesArgs URLS HEADERS)
    cmake_parse_arguments(download_distfile "${options}" "${oneValueArgs}" "${multipleValuesArgs}" ${ARGN})

    if(NOT DEFINED download_distfile_URLS)
        message(FATAL_ERROR "download_distfile requires a URLS argument.")
    endif()
    if(NOT DEFINED download_distfile_FILENAME)
        message(FATAL_ERROR "download_distfile requires a FILENAME argument.")
    endif()
    if(download_distfile_SKIP_SHA512 AND NOT VCPKG_USE_HEAD_VERSION)
        message(FATAL_ERROR "download_distfile only allows SKIP_SHA512 when building with --head")
    endif()
    if(NOT download_distfile_SKIP_SHA512 AND NOT DEFINED download_distfile_SHA512)
        message(FATAL_ERROR "download_distfile requires a SHA512 argument. If you do not know the SHA512, add it as 'SHA512 0' and re-run this command.")
    endif()
    if(download_distfile_SKIP_SHA512 AND DEFINED download_distfile_SHA512)
        message(FATAL_ERROR "download_distfile must not be passed both SHA512 and SKIP_SHA512.")
    endif()

    set(downloaded_file_path ${download_distfile_DOWNLOAD_DIR}/${download_distfile_FILENAME})
    set(download_file_path_part "${download_distfile_DOWNLOAD_DIR}/${download_distfile_FILENAME}")

    function(test_hash FILE_PATH FILE_KIND CUSTOM_ERROR_ADVICE)
        if(download_distfile_SKIP_SHA512)
            message(STATUS "Skipping hash check for ${FILE_PATH}.")
            return()
        endif()

        file(SHA512 ${FILE_PATH} FILE_HASH)
        if(NOT FILE_HASH STREQUAL download_distfile_SHA512)
            message(WARNING
                "\nFile does not have expected hash:\n"
                "        File path: [ ${FILE_PATH} ]\n"
                "    Expected hash: [ ${download_distfile_SHA512} ]\n"
                "      Actual hash: [ ${FILE_HASH} ]\n"
                "${CUSTOM_ERROR_ADVICE}\n")
            file(REMOVE ${FILE_PATH})
            set(HASH_OK 0 PARENT_SCOPE)
        else()
            set(HASH_OK 1 PARENT_SCOPE)
        endif()
    endfunction()

    if(EXISTS "${downloaded_file_path}")
        message(STATUS "Using cached ${downloaded_file_path}")
        test_hash("${downloaded_file_path}" "cached file" "Please delete the file and retry if this file should be downloaded again.")
        if(HASH_OK)
            set(${VAR} ${downloaded_file_path} PARENT_SCOPE)
        endif()
    endif()

    # If the hash check above fails the file is removed
    if(NOT EXISTS "${downloaded_file_path}")
        # Tries to download the file.
        list(GET download_distfile_URLS 0 SAMPLE_URL)
        foreach(url IN LISTS download_distfile_URLS)
            message(STATUS "Downloading ${url}...")
            if(download_distfile_HEADERS)
                foreach(header ${download_distfile_HEADERS})
                    list(APPEND request_headers HTTPHEADER ${header})
                endforeach()
            endif()
            file(DOWNLOAD ${url} "${download_file_path_part}" STATUS download_status ${request_headers})
            list(GET download_status 0 status_code)
            if (NOT "${status_code}" STREQUAL "0")
                message(STATUS "Downloading ${url}... Failed. Status: ${download_status}")
                set(download_success 0)
            else()
                set(download_success 1)
                break()
            endif()
        endforeach(url)

        if (NOT download_success)
            message(FATAL_ERROR
            "    \n"
            "    Failed to download file.\n"
            "    If you use a proxy, please set the HTTPS_PROXY and HTTP_PROXY environment\n"
            "    variables to \"https://user:password@your-proxy-ip-address:port/\".\n")
        else()
            test_hash("${download_file_path_part}" "downloaded file" "The file may have been corrupted in transit. This can be caused by proxies. If you use a proxy, please set the HTTPS_PROXY and HTTP_PROXY environment variables to \"https://user:password@your-proxy-ip-address:port/\".\n")
            get_filename_component(downloaded_file_dir "${downloaded_file_path}" DIRECTORY)
            file(MAKE_DIRECTORY "${downloaded_file_dir}")
            file(RENAME ${download_file_path_part} ${downloaded_file_path})
        endif()
    endif()
    set(${VAR} ${downloaded_file_path} PARENT_SCOPE)
    set(${_ed_NEW_FILE_VAR} 1 PARENT_SCOPE)
endfunction()
