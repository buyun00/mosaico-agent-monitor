#pragma once
#include "ambience.h"
#include "quota.h"
#include <stdint.h>
// Renderer task owns all effect particles; they never enter the quota solver.
void effects_frame(int64_t now,float surface_y,bool moving,quota_view_t quota,ambience_view_t status);
void effects_extent(int band,int *left,int *right);
void effects_draw_band(uint16_t *buf,int y0,int left,int width);
unsigned effects_completion_count(void); // diagnostic/test counter
bool effects_celebrating(void);
