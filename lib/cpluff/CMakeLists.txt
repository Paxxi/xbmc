cmake_minimum_required(VERSION 3.2)

project(cpluff VERSION 0.1.3 LANGUAGES C)

add_library(kazlib STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/kazlib/hash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/kazlib/hash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kazlib/list.c
  ${CMAKE_CURRENT_SOURCE_DIR}/kazlib/list.h
)

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

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
)

target_link_libraries(cpluff
  PRIVATE
  kazlib
  expat::expat
)

target_include_directories(cpluff
  PRIVATE 
)

target_compile_definitions(cpluff
  PRIVATE
  XML_STATIC
  _USRDLL
  CP_C_API=CP_EXPORT
  _CRT_SECURE_NO_WARNINGS
  _CRT_NONSTDC_NO_DEPRECATE
)