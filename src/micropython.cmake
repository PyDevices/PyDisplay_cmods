# Create an INTERFACE library for our C module.
add_library(usermod_pydevices INTERFACE)

# Add our source files to the lib
target_sources(usermod_pydevices INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pydevices.c
    ${CMAKE_CURRENT_LIST_DIR}/i80bus.c
    ${CMAKE_CURRENT_LIST_DIR}/spibus.c
    )

# Add the current directory as an include directory.
target_include_directories(usermod_pydevices INTERFACE
    ${IDF_PATH}/components/esp_lcd/include/
    ${CMAKE_CURRENT_LIST_DIR}
    )

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_pydevices)
