#pragma once
#include "cJSON.h"
void dashboard_init(void);
void dashboard_update(const cJSON *json);
void dashboard_gravity(float x,float y);
void dashboard_next(void);
void dashboard_frame(void);
