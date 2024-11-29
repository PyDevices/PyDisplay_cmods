# This file is to be given as 
#     make USER_C_MODULES=../../../../pydisplay_cmods/src/pydisplay.cmake
# when building Micropython.

set(CMOD_DIR ${CMAKE_CURRENT_LIST_DIR})

if(DEFINED IDF_PATH)
    # Create an INTERFACE library for our C module.
    add_library(usermod_spibus INTERFACE)
    add_library(usermod_i80bus INTERFACE)
    add_library(usermod_rgbframebuffer INTERFACE)

    # Add our source files to the lib
    target_sources(usermod_spibus INTERFACE
        ${CMOD_DIR}/busdrivers/common.c
        ${CMOD_DIR}/busdrivers/spibus.c
        )

    # Add our source files to the lib
    target_sources(usermod_i80bus INTERFACE
        ${CMOD_DIR}/busdrivers/common.c
        ${CMOD_DIR}/busdrivers/i80bus.c
        )

    # Add our source files to the lib
    target_sources(usermod_rgbframebuffer INTERFACE
        ${CMOD_DIR}/rgbframebuffer/rgbframebuffer.c
        )

    # Add include directories.
    target_include_directories(usermod_spibus INTERFACE
        ${IDF_PATH}/components/esp_lcd/include/
        ${CMOD_DIR}
        )

    # Add include directories.
    target_include_directories(usermod_i80bus INTERFACE
        ${IDF_PATH}/components/esp_lcd/include/
        ${CMOD_DIR}
        )

    # Add include directories.
    target_include_directories(usermod_rgbframebuffer INTERFACE
        ${IDF_PATH}/components/esp_lcd/include/
        ${CMOD_DIR}
        )

    # Link our INTERFACE library to the usermod target.
    target_link_libraries(usermod INTERFACE usermod_spibus usermod_i80bus usermod_rgbframebuffer)
endif()

# Create an INTERFACE library for our C module.
add_library(usermod_byteswap INTERFACE)

# Add our source files to the lib
target_sources(usermod_byteswap INTERFACE
    ${CMOD_DIR}/byteswap/byteswap.c
    )

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_byteswap)
