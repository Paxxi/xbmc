cmake_minimum_required(VERSION 3.2)

project(cpluff VERSION 0.1.3 LANGUAGES C)

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

add_library(expat::expat STATIC IMPORTED)

add_library(cpluff
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/win32/cpluffdef.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/win32/dirent.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/win32/dirent.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/win32/win32_utils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/win32/win32_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/serial.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/thread.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/thread_windows.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/util.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/util.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/context.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/cpluff.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/cpluff.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/defines.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/logging.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/pcontrol.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/pinfo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/ploader.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/pscan.c
  ${CMAKE_CURRENT_SOURCE_DIR}/libcpluff/psymbol.c
  ${CMAKE_CURRENT_SOURCE_DIR}/kazlib/hash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/kazlib/hash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kazlib/list.c
  ${CMAKE_CURRENT_SOURCE_DIR}/kazlib/list.h
)

target_link_libraries(cpluff
  PRIVATE
  expat::expat
)

target_include_directories(cpluff
  PRIVATE 
  $<BUILD_INTERFACE:kazlib;libcpluff;libcpluff/win32>
  c:/code/kodi/project/BuildDependencies/include
)

target_compile_definitions(cpluff
  PRIVATE
  XML_STATIC
  _USRDLL
  CP_C_API=CP_EXPORT
  _CRT_SECURE_NO_WARNINGS
  _CRT_NONSTDC_NO_DEPRECATE
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(EXPORT ${PROJECT_NAME}
  FILE
    ${PROJECT_NAME}.cmake
  NAMESPACE
    ${PROJECT_NAME}::
  DESTINATION
    lib/cmake/${PROJECT_NAME}
)
install(
  FILES
    cmake/${PROJECT_NAME}-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  DESTINATION
    lib/cmake/${PROJECT_NAME}
)