#include "ambience.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static SemaphoreHandle_t s_lock;
static ambience_view_t s_view = {.reset_seconds=-1};
static uint32_t s_at;
void ambience_init(void) {s_lock=xSemaphoreCreateMutex();configASSERT(s_lock);}
void ambience_update(ambience_view_t view,uint32_t now) {
    xSemaphoreTake(s_lock,portMAX_DELAY);s_view=view;s_at=now;xSemaphoreGive(s_lock);
}
ambience_view_t ambience_snapshot(uint32_t now) {
    xSemaphoreTake(s_lock,portMAX_DELAY);ambience_view_t v=s_view;uint32_t age=now-s_at;xSemaphoreGive(s_lock);
    if(v.reset_seconds>=0) v.reset_seconds=age>(uint32_t)v.reset_seconds?0:v.reset_seconds-(int)age;
    if(age>15) v.tasks_available=false;
    if(v.done_age_seconds<0x7fffffff-age) v.done_age_seconds+=age;
    return v;
}
