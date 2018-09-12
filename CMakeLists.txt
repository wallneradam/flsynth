cmake_minimum_required(VERSION 3.1)
project(flsynth C)

#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s -Wno-format-overflow -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Ofast")
SET(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -gc-sections")

if(${ANDROID})
    # We must have hardware floating point to be able to calclulate fast enough!
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math -mhard-float -mfloat-abi=softfp -mfpu=vfp -marm")
endif()

list(APPEND FL_SOURCES
        fluidlite/src/fluid_chan.c
        fluidlite/src/fluid_chorus.c
        fluidlite/src/fluid_conv.c
        fluidlite/src/fluid_defsfont.c
        fluidlite/src/fluid_dsp_float.c
        fluidlite/src/fluid_gen.c
        fluidlite/src/fluid_hash.c
        fluidlite/src/fluid_list.c
        fluidlite/src/fluid_mod.c
        fluidlite/src/fluid_ramsfont.c
        fluidlite/src/fluid_rev.c
        fluidlite/src/fluid_settings.c
        fluidlite/src/fluid_synth.c
        fluidlite/src/fluid_sys.c
        fluidlite/src/fluid_tuning.c
        fluidlite/src/fluid_voice.c
        )

if (NOT DISABLE_SF3)
    list(APPEND FL_SOURCES
            fluidlite/libvorbis-1.3.5/lib/vorbisenc.c
            fluidlite/libvorbis-1.3.5/lib/info.c
            fluidlite/libvorbis-1.3.5/lib/analysis.c
            fluidlite/libvorbis-1.3.5/lib/bitrate.c
            fluidlite/libvorbis-1.3.5/lib/block.c
            fluidlite/libvorbis-1.3.5/lib/codebook.c
            fluidlite/libvorbis-1.3.5/lib/envelope.c
            fluidlite/libvorbis-1.3.5/lib/floor0.c
            fluidlite/libvorbis-1.3.5/lib/floor1.c
            fluidlite/libvorbis-1.3.5/lib/lookup.c
            fluidlite/libvorbis-1.3.5/lib/lpc.c
            fluidlite/libvorbis-1.3.5/lib/lsp.c
            fluidlite/libvorbis-1.3.5/lib/mapping0.c
            fluidlite/libvorbis-1.3.5/lib/mdct.c
            fluidlite/libvorbis-1.3.5/lib/psy.c
            fluidlite/libvorbis-1.3.5/lib/registry.c
            fluidlite/libvorbis-1.3.5/lib/res0.c
            fluidlite/libvorbis-1.3.5/lib/sharedbook.c
            fluidlite/libvorbis-1.3.5/lib/smallft.c
            fluidlite/libvorbis-1.3.5/lib/vorbisfile.c
            fluidlite/libvorbis-1.3.5/lib/window.c
            fluidlite/libvorbis-1.3.5/lib/synthesis.c
            )

    list(APPEND LIBVORBIS_INCLUDE_DIRS
            ${CMAKE_SOURCE_DIR}/fluidlite/libvorbis-1.3.5/include
            ${CMAKE_SOURCE_DIR}/fluidlite/libvorbis-1.3.5/lib
            )

    list(APPEND FL_SOURCES
            fluidlite/libogg-1.3.2/src/bitwise.c
            fluidlite/libogg-1.3.2/src/framing.c
            )
    set(LIBOGG_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/fluidlite/libogg-1.3.2/include)

    include_directories(${LIBOGG_INCLUDE_DIRS})
    include_directories(${LIBVORBIS_INCLUDE_DIRS})
endif()

include_directories(${CMAKE_SOURCE_DIR}/fluidlite/src)
include_directories(${CMAKE_SOURCE_DIR}/fluidlite/include)

add_library(fluidlite STATIC ${FL_SOURCES})
target_compile_options(fluidlite PUBLIC -fPIC -ffunction-sections -fdata-sections)


if(${ANDROID})
    list(APPEND SL_SOURCES opensl_stream/opensl_stream.c)
    add_library(opensl STATIC ${SL_SOURCES})
endif()


# flsynth source
list(APPEND SOURCES flsynth.c flsynth.h)

add_library(flsynth SHARED ${SOURCES})
add_executable(test-cmajor tests/cmajor.c)

target_link_libraries(flsynth m)
target_link_libraries(flsynth fluidlite)
if(${ANDROID})
    target_link_libraries(flsynth opensl)
endif()
target_compile_options(flsynth PUBLIC -ffunction-sections -fdata-sections)
set_target_properties(flsynth PROPERTIES LINK_FLAGS "-Wl,--gc-sections -Wl,--version-script=${CMAKE_SOURCE_DIR}/version.script")

target_link_libraries(test-cmajor m flsynth)

if(${ANDROID})
    target_link_libraries(flsynth log OpenSLES)
    target_link_libraries(test-cmajor log OpenSLES)
else()
    target_link_libraries(flsynth SDL2)
    target_link_libraries(test-cmajor SDL2)
endif()

if (DISABLE_SF3)
    target_compile_definitions(fluidlite PUBLIC SF3_SUPPORT=0)
endif()
