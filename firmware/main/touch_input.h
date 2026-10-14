#pragma once
#include "driver/i2c_master.h"
#include "interaction.h"
esp_err_t touch_input_init(i2c_master_bus_handle_t bus);
interaction_t touch_input_poll(uint32_t ms);
