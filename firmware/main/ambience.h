#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    int reset_seconds, running, attention, failed;
    bool tasks_available;
    uint32_t event_epoch, done_seq, done_age_seconds;
} ambience_view_t;
void ambience_init(void);
void ambience_update(ambience_view_t view, uint32_t now);
ambience_view_t ambience_snapshot(uint32_t now);
