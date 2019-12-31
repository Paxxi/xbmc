#.rst:
# FindSWIG
# --------
# Finds the SWIG executable
#
# This will define the following variables::
#
# SWIG_FOUND - system has SWIG
# SWIG_EXECUTABLE - the SWIG executable

if(CMAKE_HOST_WIN32)
  set(TEMP_CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
  set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/project/BuildDependencies/native)
  find_program(SWIG_EXECUTABLE NAMES swig3.0 swig2.0 swig
                              PATHS
                                ${CMAKE_SOURCE_DIR}/project/BuildDependencies/native
                                ${CMAKE_SOURCE_DIR}/project/BuildDependencies/native/bin
                              HINTS
                                ${CMAKE_SOURCE_DIR}/project/BuildDependencies/native
                                ${CMAKE_SOURCE_DIR}/project/BuildDependencies/native/bin
                              PATH_SUFFIXES swig)
  set(CMAKE_PREFIX_PATH "${TEMP_CMAKE_PREFIX_PATH}")
else()
  find_program(SWIG_EXECUTABLE NAMES swig3.0 swig2.0 swig
                              PATH_SUFFIXES swig)
endif()
if(SWIG_EXECUTABLE)
  message(STATUS "swig: ${SWIG_EXECUTABLE}")
  execute_process(COMMAND ${SWIG_EXECUTABLE} -swiglib
    OUTPUT_VARIABLE SWIG_DIR
    ERROR_VARIABLE SWIG_swiglib_error
    RESULT_VARIABLE SWIG_swiglib_result)
  execute_process(COMMAND ${SWIG_EXECUTABLE} -version
    OUTPUT_VARIABLE SWIG_version_output
    ERROR_VARIABLE SWIG_version_output
    RESULT_VARIABLE SWIG_version_result)
    string(REGEX REPLACE ".*SWIG Version[^0-9.]*\([0-9.]+\).*" "\\1"
           SWIG_VERSION "${SWIG_version_output}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SWIG
                                  REQUIRED_VARS SWIG_EXECUTABLE SWIG_DIR
                                  VERSION_VAR SWIG_VERSION)
