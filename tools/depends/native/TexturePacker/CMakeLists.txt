if(NOT PROJECT_NAME)
  cmake_minimum_required(VERSION 3.16)
  project(TexturePacker LANGUAGES CXX)
  set(KODI_ROOT ${CMAKE_SOURCE_DIR}/../../../..)
  # separate_arguments(ARCH_DEFINES NATIVE_COMMAND ${EXTRA_DEFINES})
  set(ARCH_DEFINES ${EXTRA_DEFINES})
else()
  set(KODI_ROOT ${CMAKE_SOURCE_DIR})
endif()

list(APPEND CMAKE_MODULE_PATH ${KODI_ROOT}/cmake/modules)

if(APPLE)
  set(CMAKE_FIND_ROOT_PATH ${NATIVEPREFIX})
endif()

find_package(Lzo2 REQUIRED)
find_package(PNG REQUIRED)
find_package(GIF REQUIRED)
find_package(JPEG REQUIRED)

if(GIF_VERSION LESS 4)
  message(FATAL_ERROR "giflib < 4 not supported")
else()
  file(STRINGS ${GIF_INCLUDE_DIR}/gif_lib.h GIFSTRINGS)
  string(REGEX MATCH "GIFLIB_MAJOR ([0-9])" GIFLIB_MAJOR ${GIFSTRINGS})
  if(GIFLIB_MAJOR)
    string(REPLACE " " ";" GIFLIB_MAJOR ${GIFLIB_MAJOR})
    list(GET GIFLIB_MAJOR 1 GIFLIB_MAJOR)
  else()
    set(GIFLIB_MAJOR ${GIF_VERSION})
  endif()
  if(NOT GIFLIB_MAJOR OR GIFLIB_MAJOR LESS 5)
    message(WARNING "giflib${GIFLIB_MAJOR} support is experimental. Consider updating to giflib5")
  endif()
endif()

set(SOURCES src/md5.cpp
            src/DecoderManager.cpp
            src/TexturePacker.cpp
            src/XBTFWriter.cpp
            src/decoder/GIFDecoder.cpp
            src/decoder/GifHelper.cpp
            src/decoder/JPGDecoder.cpp
            src/decoder/PNGDecoder.cpp
            ${KODI_ROOT}/xbmc/guilib/XBTF.cpp)

set(CMAKE_POSITITION_INDEPENDENT_CODE 1)

add_executable(TexturePacker ${SOURCES})
target_include_directories(TexturePacker
                           PRIVATE ${PNG_INCLUDE_DIRS}
                                    ${JPEG_INCLUDE_DIR}
                                    ${GIF_INCLUDE_DIR}
                                    ${KODI_ROOT}/xbmc
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/decoder)

if(CMAKE_HOST_WIN32)
# The PNG module adds a dependency on shared
# zlib so we use the no_module version to specify
# a static zlib
find_package(zlib REQUIRED NO_MODULE)
find_package(libpng REQUIRED NO_MODULE)
target_link_libraries(TexturePacker
                      PRIVATE zlib::zlibstatic
                              libpng::png_static
                              ${GIF_LIBRARIES}
                              ${JPEG_LIBRARIES}
                              ${LZO2_LIBRARIES})
target_include_directories(TexturePacker
                           PRIVATE ${KODI_ROOT}/xbmc/platform/win32)
else()
target_link_libraries(TexturePacker
                      PRIVATE ${SYSTEM_LDFLAGS}
                              ${GIF_LIBRARIES}
                              ${PNG_LIBRARIES}
                              ${JPEG_LIBRARIES}
                              ${LZO2_LIBRARIES})
endif()
target_compile_options(TexturePacker PRIVATE ${ARCH_DEFINES})

export(TARGETS TexturePacker
               NAMESPACE TexturePacker::
               FILE TexturePacker.cmake)