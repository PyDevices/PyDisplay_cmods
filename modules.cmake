# Edit this file to include all user c modules you want in the build.
# This file is to be given as
#     make USER_C_MODULES=../../../../pydisplay_cmods/micropython.cmake


include(${CMAKE_CURRENT_LIST_DIR}/src/pydisplay.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../lvmp/lvmp.cmake)
