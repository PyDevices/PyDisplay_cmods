# When building Micropython, this file is to be given as:
#   for esp32:
#     make USER_C_MODULES=../../../../pydisplay_cmods/micropython.cmake
#   for rp2 and most other (CMake-based) ports:
#     make USER_C_MODULES=../../../pydisplay_cmods/micropython.cmake

set(CMOD_DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(usermod_pydisplay INTERFACE)

target_sources(usermod_pydisplay INTERFACE
    ${CMOD_DIR}/src/byteswap/byteswap.c
    )

target_include_directories(usermod_pydisplay INTERFACE
    ${CMOD_DIR}
    )

if(DEFINED IDF_PATH)
    target_sources(usermod_pydisplay INTERFACE
        ${CMOD_DIR}/src/buses/common/common.c
        ${CMOD_DIR}/src/buses/esp32/spibus.c
        ${CMOD_DIR}/src/buses/esp32/i80bus.c
        ${CMOD_DIR}/src/rgbframebuffer/esp32/rgbframebuffer.c
        )

    target_include_directories(usermod_pydisplay INTERFACE
        ${IDF_PATH}/components/esp_lcd/include/
        )
endif()

target_link_libraries(usermod INTERFACE usermod_pydisplay)
