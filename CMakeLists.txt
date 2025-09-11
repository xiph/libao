cmake_minimum_required(VERSION 3.20)
project(libao)

set(SRC
    src/ao_aixs.c
    src/ao_au.c
    src/ao_null.c
    src/ao_raw.c
    src/ao_wav.c
    src/audio_out.c
    src/config.c)
if(WIN32)
    list(APPEND SRC src/ao_wmm.c)
endif()
configure_file(include/ao/os_types.h.in include/os_types.h)

add_library(ao ${SRC})
set_property(TARGET ao PROPERTY C_STANDARD 17)
set_property(TARGET ao PROPERTY CPP_STANDARD 17)
target_include_directories(ao
    PUBLIC
        include
        ${CMAKE_BINARY_DIR}/include
    PRIVATE
        include/ao)

target_compile_definitions(ao
    PRIVATE
        AO_BUILDING_LIBAO)

if(WIN32)
    target_compile_definitions(ao PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()