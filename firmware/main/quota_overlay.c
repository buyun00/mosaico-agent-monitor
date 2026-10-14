#include "quota_overlay.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include "ambience.h"
#include "esp_timer.h"
#include "effects.h"

// Original 5x7 bitmap glyphs, row-major, scaled for a crisp small HUD.
static const char alphabet[] = "0123456789% CODEXWKLIUVBSTA-FPN.RYHM:";
static const uint8_t glyphs[][7] = {
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
 {30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
 {14,17,17,15,1,1,14},{25,25,2,4,8,19,19},{0,0,0,0,0,0,0},
 {14,17,16,16,16,17,14},{14,17,17,17,17,17,14},{30,17,17,17,17,17,30},
 {31,16,16,30,16,16,31},{17,17,10,4,10,17,17},{17,17,17,21,21,21,10},
 {17,18,20,24,20,18,17},{16,16,16,16,16,16,31},{14,4,4,4,4,4,14},
 {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{30,17,17,30,17,17,30},
 {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{14,17,17,31,17,17,17},
 {0,0,0,31,0,0,0},{31,16,16,30,16,16,16},{30,17,17,30,16,16,16},
 {17,25,25,21,19,19,17},{0,0,0,0,0,12,12},
 {30,17,17,30,20,18,17},{17,17,10,4,4,4,4},{17,17,17,31,17,17,17},
 {17,27,21,21,17,17,17},{0,12,12,0,12,12,0}
};
static uint16_t wire(uint16_t rgb) { return (rgb >> 8) | (rgb << 8); }
static void text(uint16_t *buf, int y0, int left, int width,
                 int x, int y, const char *str, int scale, uint16_t color)
{
    for (; *str; ++str, x += 6 * scale) {
        const char *entry = strchr(alphabet, *str);
        if (!entry) continue;
        const uint8_t *bits = glyphs[entry - alphabet];
        for (int row = 0; row < 7 * scale; ++row) {
            int py = y + row;
            if (py < y0 || py >= y0 + BAND_ROWS) continue;
            for (int col = 0; col < 5 * scale; ++col) {
                int px = x + col;
                if (px >= left && px < left + width && (bits[row / scale] & (16 >> (col / scale))))
                    buf[(py - y0) * width + px - left] = wire(color);
            }
        }
    }
}
void quota_draw_band(uint16_t *buf, int y0, int left, int width, quota_view_t view)
{
    for (int y = y0; y < y0 + BAND_ROWS; ++y) {
        if (y < QUOTA_TOP || y >= QUOTA_BOTTOM) continue;
        for (int x = QUOTA_LEFT; x < QUOTA_RIGHT; ++x)
            if (x >= left && x < left + width) buf[(y-y0)*width+x-left] = wire(0x0863);
    }
    char amount[16];
    unsigned pin = quota_setup_pin();
    if (pin) {
        char pin_text[16];
        snprintf(pin_text,sizeof pin_text,"PIN %08u",pin);
        text(buf,y0,left,width,76,68,"WIFI SETUP",2,0xffff);
        text(buf,y0,left,width,76,90,"FLUIDBOX-SETUP",2,0x77fa);
        text(buf,y0,left,width,76,112,pin_text,2,0xffff);
        text(buf,y0,left,width,76,134,"192.168.4.1",2,0xffff);
        return;
    }
    if (view.percent < 0) snprintf(amount, sizeof amount, "--");
    else snprintf(amount, sizeof amount, "%d%%", view.percent);
    uint16_t color = view.stale ? 0xff20 : view.percent <= 20 ? 0xfba0 : 0x77fa;
    text(buf,y0,left,width,76,68,"CODEX WEEK",2,0xffff);
    text(buf,y0,left,width,76,90,amount,5,color);
    ambience_view_t a=ambience_snapshot((uint32_t)(esp_timer_get_time()/1000000));
    char status[32];
    if(view.stale)snprintf(status,sizeof status,"WIFI --");
    else if(!a.tasks_available)snprintf(status,sizeof status,"TASKS --");
    else if(a.attention)snprintf(status,sizeof status,"REPLY %d",a.attention);
    else if(a.failed)snprintf(status,sizeof status,"FAILED %d",a.failed);
    else if(a.running)snprintf(status,sizeof status,"WORK %d",a.running);
    else if(effects_celebrating())snprintf(status,sizeof status,"DONE");
    else snprintf(status,sizeof status,"IDLE");
    text(buf,y0,left,width,76,134,status,2,a.attention?0xfea0:color);
    char reset[32];
    if(a.reset_seconds<0)snprintf(reset,sizeof reset,"RESET --");
    else if(a.reset_seconds>=86400)snprintf(reset,sizeof reset,"%dD %02dH RESET",a.reset_seconds/86400,a.reset_seconds/3600%24);
    else snprintf(reset,sizeof reset,"%02dH %02dM RESET",a.reset_seconds/3600,a.reset_seconds/60%60);
    text(buf,y0,left,width,76,164,reset,2,view.stale?0x7bef:0xbdf7);
}
