#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {bool down,swirl,double_tap;float x,y,dx,dy;} interaction_t;
// Called only by simulation task. Coordinates are display pixels.
interaction_t interaction_update(bool down,float x,float y,uint32_t ms);
// Renderer reads a packed atomic view; no gesture state is mutated.
interaction_t interaction_snapshot(void);
