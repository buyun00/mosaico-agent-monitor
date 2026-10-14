cmake_minimum_required(VERSION 3.16)
set(IDF_TARGET esp32s31)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(mosaico-agent-monitor VERSION 0.15.0)
# Register the companion resource image so a future explicit flash cannot omit it.
esptool_py_flash_to_partition(flash miku_assets "${CMAKE_CURRENT_LIST_DIR}/../assets/miku-animation.bin")
