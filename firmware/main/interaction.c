#include "interaction.h"
#include <math.h>
#include <stdatomic.h>
static atomic_uint s_display;
interaction_t interaction_snapshot(void) {
    unsigned v=atomic_load(&s_display);
    return (interaction_t){.down=(v>>20)&1,.swirl=(v>>21)&1,.x=v&1023,.y=(v>>10)&1023};
}
interaction_t interaction_update(bool down,float x,float y,uint32_t ms) {
    static bool held,moved,tapped;
    static float start_x,start_y,last_x,last_y,tap_x,tap_y;
    static uint32_t began,last_down,last_tap;
    interaction_t out={.x=x,.y=y};
    if(x<0)x=0;
    if(x>479)x=479;
    if(y<0)y=0;
    if(y>479)y=479;
    if(down)last_down=ms;
    if(!down&&held&&ms-last_down<80){down=true;x=last_x;y=last_y;}
    if(down&&!held){began=ms;start_x=last_x=x;start_y=last_y=y;moved=false;}
    if(down){
        if((x-start_x)*(x-start_x)+(y-start_y)*(y-start_y)>18*18)moved=true;
        out.dx=fmaxf(-40,fminf(40,x-last_x));out.dy=fmaxf(-40,fminf(40,y-last_y));
        out.swirl=!moved&&ms-began>=600;
        last_x=x;last_y=y;
    } else if(held&&!moved&&ms-began<350){
        x=last_x;y=last_y;
        if(tapped&&ms-last_tap<400&&(x-tap_x)*(x-tap_x)+(y-tap_y)*(y-tap_y)<40*40){out.double_tap=true;tapped=false;}
        else {tapped=true;last_tap=ms;tap_x=last_x;tap_y=last_y;}
    }
    out.down=down;out.x=x;out.y=y;held=down;
    atomic_store(&s_display,(unsigned)x|((unsigned)y<<10)|((unsigned)down<<20)|((unsigned)out.swirl<<21));
    return out;
}
