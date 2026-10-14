#pragma once
#ifdef ESP_PLATFORM
#include "esp_attr.h"
#define FLUID_LARGE_BUFFER EXT_RAM_BSS_ATTR
#else
#define FLUID_LARGE_BUFFER
#endif
