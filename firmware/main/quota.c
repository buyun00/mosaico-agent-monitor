#include "quota.h"
#include "config.h"
#include <stdatomic.h>
#include <string.h>

// One atomic word publishes timestamp and percentage together. 24-bit seconds
// wrap after 194 days; subtraction is modulo that same range.
static atomic_uint s_sample = 127; // unknown, deliberately not zero or full
static atomic_uint s_setup_pin;
void quota_set_setup_pin(unsigned pin) { atomic_store(&s_setup_pin, pin); }
unsigned quota_setup_pin(void) { return atomic_load(&s_setup_pin); }

bool quota_accept_line(const char *line, uint32_t now_seconds)
{
    if (strncmp(line, "FBQ1 ", 5)) return false;
    const char *p = line + 5;
    if (!strcmp(p, "-1")) {
        atomic_fetch_or(&s_sample, 128u);
        return true;
    }
    int value = 0, digits = 0;
    for (; *p; ++p) {
        if (*p < '0' || *p > '9' || ++digits > 3) return false;
        value = value * 10 + *p - '0';
    }
    if (!digits || value > 100) return false;
    atomic_store(&s_sample, ((now_seconds & 0xffffffu) << 8) | (unsigned)value);
    return true;
}

quota_view_t quota_snapshot(uint32_t now_seconds)
{
    unsigned sample = atomic_load(&s_sample);
    int percent = sample & 127u;
    uint32_t age = (now_seconds - (sample >> 8)) & 0xffffffu;
    return (quota_view_t){percent <= 100 ? percent : -1,
                         (sample & 128u) || percent > 100 || age > 90};
}

int quota_particle_count(int percent)
{
    if (percent < 0 || percent > 100) return -1;
    return (percent * PARTICLE_MAX + 50) / 100;
}
