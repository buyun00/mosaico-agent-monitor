#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct { int percent; bool stale; } quota_view_t;
// Internal sample format: FBQ1 <remaining-percent>; -1 marks last sample stale.
// Strict bounded parser; safe to call from the network task.
bool quota_accept_line(const char *line, uint32_t now_seconds);
quota_view_t quota_snapshot(uint32_t now_seconds);
int quota_particle_count(int percent);
void quota_set_setup_pin(unsigned pin);
unsigned quota_setup_pin(void);
