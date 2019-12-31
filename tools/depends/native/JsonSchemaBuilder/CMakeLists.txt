if(NOT PROJECT_NAME)
  cmake_minimum_required(VERSION 3.16)
  project(JsonSchemaBuilder LANGUAGES CXX)
  set(KODI_ROOT ${CMAKE_SOURCE_DIR}/../../../..)
  set(ARCH_DEFINES ${EXTRA_DEFINES})
else()
  set(KODI_ROOT ${CMAKE_SOURCE_DIR})
endif()
set(SOURCES src/JsonSchemaBuilder.cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(JsonSchemaBuilder ${SOURCES})

export(TARGETS JsonSchemaBuilder
               NAMESPACE JsonSchemaBuilder::
               FILE JsonSchemaBuilder.cmake)