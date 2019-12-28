## # extract_distfile
##
## Extract an archive into the source directory.
##
## ## Usage
## ```cmake
## extract_distfile(
##     OUT_SOURCE_PATH <SOURCE_PATH>
##     ARCHIVE <${ARCHIVE}>
##     [REF <1.0.0>]
##     [NO_REMOVE_ONE_LEVEL]
##     [WORKING_DIRECTORY <${CURRENT_BUILDTREES_DIR}/src>]
## )
## ```
## ## Parameters
## ### SOURCE_PATH
## Specifies the directory the files should be extracted to
##
## ### ARCHIVE
## The full path to the archive to be extracted.
##
## This is usually obtained from calling `download_distfile`
##
## ### REF
## A friendly name that will be used instead of the filename of the archive.  If more than 10 characters it will be truncated.
##
## By convention, this is set to the version number or tag fetched
##
## ### WORKING_DIRECTORY
## If specified, the archive will be extracted into the working directory instead of `${CURRENT_BUILDTREES_DIR}/src/`.
##
## Note that the archive will still be extracted into a subfolder underneath that directory (`${WORKING_DIRECTORY}/${REF}-${HASH}/`).
##
## ### NO_REMOVE_ONE_LEVEL
## Specifies that the default removal of the top level folder should not occur.
##
include(cmake/scripts/common/ExecuteRequiredProcess.cmake)
function(extract_distfile)
    cmake_parse_arguments(_ed "NO_REMOVE_ONE_LEVEL" "FORCE;SOURCE_PATH;ARCHIVE;WORKING_DIRECTORY" "" ${ARGN})

    if(NOT _ed_ARCHIVE)
        message(FATAL_ERROR "Must specify ARCHIVE parameter to extract_distfile()")
    endif()

    if(NOT DEFINED _ed_SOURCE_PATH)
        message(FATAL_ERROR "Must specify SOURCE_PATH parameter to extract_distfile()")
    endif()

    if(NOT DEFINED _ed_WORKING_DIRECTORY)
        message(FATAL_ERROR "Must specify WORKING_DIRECTORY parameter to extract_distfile()")
    endif()

    set(TEMP_DIR "${_ed_WORKING_DIRECTORY}/TEMP")
    file(REMOVE_RECURSE ${TEMP_DIR})

    get_filename_component(ARCHIVE_FILENAME "${_ed_ARCHIVE}" NAME)
    if(_ed_FORCE OR NOT EXISTS ${_ed_SOURCE_PATH}/${ARCHIVE_FILENAME}.extracted)
        message(STATUS "Extracting source ${_ed_ARCHIVE}")
        file(MAKE_DIRECTORY ${TEMP_DIR})
        execute_required_process(
            COMMAND ${CMAKE_COMMAND} -E tar xjf ${_ed_ARCHIVE}
            WORKING_DIRECTORY ${TEMP_DIR}
            LOGNAME extract
        )
        file(WRITE ${_ed_SOURCE_PATH}/${ARCHIVE_FILENAME}.extracted)

        if(_ed_NO_REMOVE_ONE_LEVEL)
            set(TEMP_SOURCE_PATH ${TEMP_DIR})
        else()
            file(GLOB _ARCHIVE_FILES "${TEMP_DIR}/*")
            list(LENGTH _ARCHIVE_FILES _NUM_ARCHIVE_FILES)
            set(TEMP_SOURCE_PATH)
            foreach(dir IN LISTS _ARCHIVE_FILES)
                if (IS_DIRECTORY ${dir})
                    set(TEMP_SOURCE_PATH "${dir}")
                    break()
                endif()
            endforeach()

            if(NOT _NUM_ARCHIVE_FILES EQUAL 1 OR NOT TEMP_SOURCE_PATH)
                message(FATAL_ERROR "Could not unwrap top level directory from archive. Pass NO_REMOVE_ONE_LEVEL to disable this.")
            endif()
        endif()

        file(COPY ${TEMP_SOURCE_PATH}/ DESTINATION ${_ed_SOURCE_PATH})
        file(REMOVE_RECURSE ${TEMP_DIR})
    endif()

    return()
endfunction()