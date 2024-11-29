# When building Micropython this file is to be given as:
#     make USER_C_MODULES=../../../../pydisplay_cmods/src/pydisplay.cmake

set(CMOD_DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(usermod_pydisplay INTERFACE)

target_sources(usermod_pydisplay INTERFACE
    ${CMOD_DIR}/byteswap/byteswap.c
    )

target_include_directories(usermod_pydisplay INTERFACE
    ${CMOD_DIR}
    )

if(DEFINED IDF_PATH)
    target_sources(usermod_pydisplay INTERFACE
        ${CMOD_DIR}/busdrivers/common.c
        ${CMOD_DIR}/busdrivers/spibus.c
        ${CMOD_DIR}/busdrivers/i80bus.c
        ${CMOD_DIR}/rgbframebuffer/rgbframebuffer.c
        )

    target_include_directories(usermod_pydisplay INTERFACE
        ${IDF_PATH}/components/esp_lcd/include/
        )
endif()

target_link_libraries(usermod INTERFACE usermod_pydisplay)
