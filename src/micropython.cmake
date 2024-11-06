# Create an INTERFACE library for our C module.
add_library(usermod_spibus INTERFACE)
add_library(usermod_i80bus INTERFACE)
add_library(usermod_rgbframebuffer INTERFACE)

# Add our source files to the lib
target_sources(usermod_spibus INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/common.c
    ${CMAKE_CURRENT_LIST_DIR}/spibus.c
    )

# Add our source files to the lib
target_sources(usermod_i80bus INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/common.c
    ${CMAKE_CURRENT_LIST_DIR}/i80bus.c
    )

# Add our source files to the lib
target_sources(usermod_rgbframebuffer INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/common.c
    ${CMAKE_CURRENT_LIST_DIR}/rgbframebuffer.c
    )

# Add the current directory as an include directory.
target_include_directories(usermod_spibus INTERFACE
    ${IDF_PATH}/components/esp_lcd/include/
    ${CMAKE_CURRENT_LIST_DIR}
    )

# Add the current directory as an include directory.
target_include_directories(usermod_i80bus INTERFACE
    ${IDF_PATH}/components/esp_lcd/include/
    ${CMAKE_CURRENT_LIST_DIR}
    )

# Add the current directory as an include directory.
target_include_directories(usermod_rgbframebuffer INTERFACE
    ${IDF_PATH}/components/esp_lcd/include/
    ${CMAKE_CURRENT_LIST_DIR}
    )

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_spibus usermod_i80bus usermod_rgbframebuffer)
