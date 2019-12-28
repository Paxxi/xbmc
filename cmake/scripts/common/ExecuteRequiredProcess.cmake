## # execute_required_process
##
## Execute a process with logging and fail the build if the command fails.
##
## ## Usage
## ```cmake
## execute_required_process(
##     COMMAND <${PERL}> [<arguments>...]
##     WORKING_DIRECTORY <${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg>
##     LOGNAME <build-${TARGET_TRIPLET}-dbg>
## )
## ```
## ## Parameters
## ### COMMAND
## The command to be executed, along with its arguments.
##
## ### WORKING_DIRECTORY
## The directory to execute the command in.
##
## ### LOGNAME
## The prefix to use for the log files.
##
## This should be a unique name for different triplets so that the logs don't conflict when building multiple at once.
function(execute_required_process)
    cmake_parse_arguments(execute_required_process "" "WORKING_DIRECTORY;LOGNAME" "COMMAND" ${ARGN})
    set(LOG_OUT "${PROJECT_BINARY_DIR}/${execute_required_process_LOGNAME}-out.log")
    set(LOG_ERR "${PROJECT_BINARY_DIR}/${execute_required_process_LOGNAME}-err.log")

    execute_process(
        COMMAND ${execute_required_process_COMMAND}
        OUTPUT_FILE ${LOG_OUT}
        ERROR_FILE ${LOG_ERR}
        RESULT_VARIABLE error_code
        WORKING_DIRECTORY ${execute_required_process_WORKING_DIRECTORY})
    if(error_code)
        set(LOGS)
        file(READ "${LOG_OUT}" out_contents)
        file(READ "${LOG_ERR}" err_contents)
        if(out_contents)
            list(APPEND LOGS "${LOG_OUT}")
        endif()
        if(err_contents)
            list(APPEND LOGS "${LOG_ERR}")
        endif()
        set(STRINGIFIED_LOGS)
        foreach(LOG ${LOGS})
            file(TO_NATIVE_PATH "${LOG}" NATIVE_LOG)
            list(APPEND STRINGIFIED_LOGS "    ${NATIVE_LOG}\n")
        endforeach()
        prettify_command(execute_required_process_COMMAND execute_required_process_COMMAND_PRETTY)
        message(FATAL_ERROR
            "  Command failed: ${execute_required_process_COMMAND_PRETTY}\n"
            "  Working Directory: ${execute_required_process_WORKING_DIRECTORY}\n"
            "  Error code: ${error_code}\n"
            "  See logs for more information:\n"
            ${STRINGIFIED_LOGS}
        )
    endif()
endfunction()

## # prettify_command
##
## Turns list of command arguments into a formatted string.
##
## ## Usage
## ```cmake
## prettify_command()
## ```
##
macro(prettify_command INPUT_VAR OUTPUT_VAR)
    set(${OUTPUT_VAR} "")
    foreach(v ${${INPUT_VAR}})
        if(${v} MATCHES "( )")
            list(APPEND ${OUTPUT_VAR} \"${v}\")
        else()
            list(APPEND ${OUTPUT_VAR} ${v})
        endif()
    endforeach()
    list(JOIN ${OUTPUT_VAR} " " ${OUTPUT_VAR})
endmacro()