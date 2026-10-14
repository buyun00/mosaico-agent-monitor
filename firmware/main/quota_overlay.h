#pragma once
#include "quota.h"
#include <stdint.h>
// Inset from the rounded upper-left corner. Coordinates are panel pixels.
#define QUOTA_LEFT 64
#define QUOTA_RIGHT 268
#define QUOTA_TOP 60
#define QUOTA_BOTTOM 186
void quota_draw_band(uint16_t *pixels, int y0, int left, int width, quota_view_t view);
