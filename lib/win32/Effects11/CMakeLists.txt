set(SOURCES d3dxGlobal.cpp
            EffectAPI.cpp
            EffectLoad.cpp
            EffectNonRuntime.cpp
            EffectReflection.cpp
            EffectRuntime.cpp)

add_library(effects11 STATIC ${SOURCES})
target_include_directories(effects11
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Binary
)
add_precompiled_header(effects11 pchfx.h d3dxGlobal.cpp)
set_target_properties(effects11 PROPERTIES FOLDER lib)
source_group_by_folder(effects11)
set(core_DEPENDS effects11 ${core_DEPENDS} CACHE STRING "" FORCE)
