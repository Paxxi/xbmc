function(build_native_tool)
  set(options)
  set(oneValueArgs NAME SOURCE_DIR BUILD_DIR)
  set(multiValueArgs EXTRA_ARGS WORKING_DIRECTORY)
  cmake_parse_arguments(NATIVE_TOOL "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  list(JOIN ARCH_DEFINES ";" NATIVE_TOOL_DEFINES)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
                  "-DEXTRA_DEFINES=${NATIVE_TOOL_DEFINES}"
                  ${NATIVE_TOOL_EXTRA_ARGS}
                  -S ${NATIVE_TOOL_SOURCE_DIR}
                  -B ${NATIVE_TOOL_BUILD_DIR}
                  WORKING_DIRECTORY ${NATIVE_TOOL_WORKING_DIRECTORY}
                  COMMAND_ECHO STDOUT)
  execute_process(COMMAND ${CMAKE_COMMAND} --build ${NATIVE_TOOL_BUILD_DIR} --config RelWithDebInfo
                  WORKING_DIRECTORY ${NATIVE_TOOL_WORKING_DIRECTORY}
                  COMMAND_ECHO STDOUT)

  find_program(NATIVE_TOOL_EXECUTABLE ${NATIVE_TOOL_NAME}
                HINTS ${NATIVE_TOOL_BUILD_DIR}
                PATH_SUFFIXES ${CMAKE_CONFIGURATION_TYPES})

  if(NATIVE_TOOL_EXECUTABLE)
    string(TOUPPER ${NATIVE_TOOL_NAME} NATIVE_TOOL_UC_NAME)
    get_filename_component(NATIVE_TOOL_DIRECTORY ${NATIVE_TOOL_EXECUTABLE} DIRECTORY)
    set(${NATIVE_TOOL_UC_NAME}_DIR ${NATIVE_TOOL_DIRECTORY} PARENT_SCOPE)
  endif()

  unset(NATIVE_TOOL_EXECUTABLE CACHE)
endfunction(build_native_tool)