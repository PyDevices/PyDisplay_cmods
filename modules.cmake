# Edit this file to include all user c modules you want in the build.
# This file is to be given as
#     make USER_C_MODULES=../../../../busdrivers/micropython.cmake


include(${CMAKE_CURRENT_LIST_DIR}/src/pydisplay.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../lv_binding_micropython/lvgl.cmake)
