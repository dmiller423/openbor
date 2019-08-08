cmake_minimum_required(VERSION 3.4.0)
cmake_policy(SET CMP0065 NEW)

project(openbor)
enable_language(ASM)

set(CMAKE_INCLUDE_CURRENT_DIRECTORY On)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)


if (MSVC OR "${PLATFORM}" STREQUAL "WIN")
	add_definitions(-DWIN)
endif()



if (NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-writable-strings -Wno-tentative-definition-incomplete-type -Wno-implicit-function-declaration")
endif()

if (PS4)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/engine/ps4)
endif()


set(d_engine "${CMAKE_CURRENT_SOURCE_DIR}/engine")
set(d_src "${d_engine}/source")

include_directories("${d_engine}")
include_directories("${d_src}")
include_directories("${d_src}/gamelib")
include_directories("${d_src}/scriptlib")
include_directories("${d_src}/openborscript")
include_directories("${d_src}/preprocessorlib")
include_directories("${d_src}/adpcmlib")
include_directories("${d_src}/randlib")		# 
include_directories("${d_src}/ramlib")		# 
include_directories("${d_src}/gfxlib")		# bc authors were retarded apparently
include_directories("${d_src}/pnglib")		# Is there a settings to just include every fucking subdirectory
include_directories("${d_src}/webmlib")		# It just looks optional,  or else you manually supply vorbislib anyhow .. makes tons of sense



add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/ogg)

set(OGG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/ext/ogg/include)
set(OGG_LIBRARIES ogg)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/vorbis)


include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/ogg/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/vorbis/include")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/libsamplerate/src")



include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/zlib")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/libpng")

file(GLOB S_zlib ${CMAKE_CURRENT_SOURCE_DIR}/ext/zlib/*.c)
file(GLOB S_lpng ${CMAKE_CURRENT_SOURCE_DIR}/ext/libpng/*.c)

set(S_ext ${S_zlib} ${S_lpng})


set(S_adpcm ${d_src}/adpcmlib/adpcm.c)
file(GLOB S_gfx ${d_src}/gfxlib/*.c)
file(GLOB S_game ${d_src}/gamelib/*.c)
file(GLOB S_script ${d_src}/scriptlib/*.c ${d_src}/preprocessorlib/*.c ${d_src}/openborscript/*.c)
file(GLOB S_misc ${d_src}/ramlib/*.c ${d_src}/randlib/*.c ${d_src}/pnglib/*.c)
file(GLOB S_main ${d_engine}/*.c ${d_src}/*.c)


set(SRCS ${S_game} ${S_main} ${S_script} ${S_misc} ${S_gfx} ${S_adpcm} ${S_ext})



if (PS4)
	set(d_ps4 ${d_engine}/ps4)
	set(d_imgui ${d_ps4}/imgui)

	set(S_imgui ${d_imgui}/imgui_sw.cpp ${d_imgui}/imgui.cpp ${d_imgui}/imgui_draw.cpp ${d_imgui}/imgui_widgets.cpp)
	set(S_ps4	${d_ps4}/ps4-main.cpp ${d_ps4}/util.s ${d_ps4}/sblaster.cpp ${d_ps4}/control.cpp ${S_imgui})
	list(APPEND SRCS ${S_ps4})
endif()


add_executable(openbor${binSuffix} ${SRCS})


target_link_libraries(openbor${binSuffix} "ogg"  "vorbis" "vorbisfile")