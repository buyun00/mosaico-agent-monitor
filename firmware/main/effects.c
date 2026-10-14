#include "effects.h"
#include "interaction.h"
#include "config.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define TAU 6.283185307f
#define DOTS 144
#define SHAPES 640
typedef struct {float x,y,vx,vy,life,maxlife;uint16_t color;int kind;} dot_t;
typedef struct {int x,y,rx,ry;uint16_t color;uint8_t alpha;bool ring,dirty;} shape_t;
static dot_t dots[DOTS];
static shape_t shapes[SHAPES];
static int shape_count,next_dot;
static int64_t last_frame,last_motion,last_spawn,last_quota;
static int64_t celebration,next_pop;
static uint32_t rng=0x1234abcd,epoch,seq;
static int old_percent=-1,old_failed,old_second=-1;
static unsigned completions;
static int pops_left;
static bool baseline;
unsigned effects_completion_count(void){return completions;}
bool effects_celebrating(void){return last_frame<celebration;}
static float random01(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return (rng&65535)/65535.0f;}
static void shape(float x,float y,float rx,float ry,uint16_t color,int alpha,bool ring,bool dirty){
    if(shape_count>=SHAPES||alpha<=0||rx<1||ry<1)return;
    shapes[shape_count++]=(shape_t){(int)x,(int)y,(int)rx,(int)ry,color,(uint8_t)(alpha>255?255:alpha),ring,dirty};
}
static void emit(float x,float y,float vx,float vy,float life,uint16_t color,int kind){
    dots[next_dot++%DOTS]=(dot_t){x,y,vx,vy,life,life,color,kind};
}
static void burst(float x,float y,int count){
    static const uint16_t palette[]={0x77ff,0xf81f,0xff20,0x5fe9,0xfa89,0x9c7f};
    for(int i=0;i<count;++i){float a=random01()*TAU,s=45+random01()*150;emit(x,y,cosf(a)*s,sinf(a)*s,1.8f+random01()*1.5f,palette[i%6],0);}
}
static void border(ambience_view_t status,int nowsec){
    if(status.reset_seconds<0)return;
    float fraction=fminf(1,status.reset_seconds/604800.0f);
    bool dirty=old_second!=nowsec;old_second=nowsec;
    // Rounded frame, inset from the physical corners; clockwise weekly timer.
    const int segments=160;
    for(int i=0;i<segments;++i){
        float a=-TAU/4+TAU*i/segments;
        float c=cosf(a),s=sinf(a);
        float x=240+220*copysignf(powf(fabsf(c),0.5f),c);
        float y=240+220*copysignf(powf(fabsf(s),0.5f),s);
        shape(x,y,2,2,i<(int)(fraction*segments)?0xcfff:0x2945,190,false,dirty);
    }
}
void effects_frame(int64_t now,float surface_y,bool moving,quota_view_t quota,ambience_view_t status){
    float dt=last_frame?fminf((now-last_frame)/1000000.0f,0.1f):0;
    last_frame=now;shape_count=0;
    float t=now/1000000.0f;
    interaction_t touch=interaction_snapshot();
    if(moving||touch.down||!last_motion)last_motion=now;
    bool calm=now-last_motion>15000000;
    bool live=status.tasks_available&&!quota.stale;
    if(!baseline||epoch!=status.event_epoch){epoch=status.event_epoch;seq=status.done_seq;baseline=status.event_epoch!=0;}
    else if(live&&seq!=status.done_seq){
        if(status.done_seq>seq&&status.done_age_seconds<30){celebration=now+4500000;next_pop=now;pops_left=3;completions++;}
        seq=status.done_seq;
    }
    if(pops_left&&now>=next_pop){burst(140+random01()*200,190+random01()*100,42);next_pop=now+850000;pops_left--;}
    if(live&&status.failed>old_failed)for(int i=0;i<12;i++)emit(240,240,(random01()-.5f)*90,-30-random01()*50,1.5f,0xf986,1);
    old_failed=live?status.failed:0;
    if(quota.percent>=0){
        if(old_percent>=0&&!quota.stale&&quota.percent!=old_percent){
            int change=quota.percent-old_percent;int amount=abs(change)*3+12;if(amount>45)amount=45;
            for(int i=0;i<amount;i++){
                float x=65+random01()*350;
                if(change>0)emit(x,35+random01()*60,0,90+random01()*60,1.6f,0x7ffb,2);
                else emit(x,surface_y+random01()*70,(random01()-.5f)*30,-50-random01()*50,1.8f,0xafff,1);
            }
            last_quota=now;
        }
        old_percent=quota.percent;
    }
    if(live&&status.running>0){
        // Several expanding elliptical ripples skim the real fluid surface.
        for(int i=0;i<3;i++){
            float phase=fmodf(t*.48f+i/3.0f,1);
            float x=240+70*sinf(t*.35f+i*1.7f);
            shape(x,surface_y+8+i*6,15+phase*110,3+phase*16,0x5e9f,(int)((1-phase)*190),true,true);
        }
        for(int i=0;i<3;i++)shape(300+i*12,146,2,2,0x7fff,(int)(90+120*(.5f+.5f*sinf(t*3-i))),false,true);
    }
    if(live&&status.attention>0){
        int bubbles=status.attention+3;if(bubbles>8)bubbles=8;
        for(int i=0;i<bubbles;i++){
            float p=fmodf(t*.16f+i/(float)bubbles,1);
            float x=290+105*sinf(i*2.4f+p*2),y=425-p*245,r=7+8*sinf(p*3.14159f);
            shape(x,y,r+3,r+3,0xfdc0,30,false,true);
            shape(x,y,r,r,0xfea0,220,true,true);
            shape(x-r*.35f,y-r*.4f,2,2,0xffff,230,false,true);
            if(i==0){shape(x,y-2,1,4,0xffff,240,false,true);shape(x,y+6,1,1,0xffff,240,false,true);}
        }
    } else if(calm&&!quota.stale&&quota.percent>0){
        for(int i=0;i<3;i++){
            float p=fmodf(t*.09f+i/3.0f,1);
            shape(110+i*115+8*sinf(t+i),460-p*(460-surface_y),3+p*3,3+p*3,0x75db,75,true,true);
        }
        float p=fmodf(t*.16f,1);
        shape(260,surface_y+7,8+p*65,2+p*9,0x6497,(int)((1-p)*70),true,true);
    }
    if(live&&status.failed){float p=fmodf(t*.6f,1);shape(240,240,40+p*90,40+p*90,0xf9e7,(int)((1-p)*110),true,true);}
    if(touch.down){
        shape(touch.x,touch.y,12,12,0xbfff,190,true,true);
        if(touch.swirl)for(int i=0;i<16;i++){float a=t*4+i*.45f,r=10+i*3;shape(touch.x+cosf(a)*r,touch.y+sinf(a)*r,2,2,0x8fff,230-i*9,false,true);}
    }
    for(int i=0;i<DOTS;i++)if(dots[i].life>0){
        dot_t *d=&dots[i];d->life-=dt;if(d->life<=0)continue;
        d->x+=d->vx*dt;d->y+=d->vy*dt;if(d->kind==0)d->vy+=65*dt;
        int alpha=(int)(255*fminf(1,d->life*.8f));
        shape(d->x,d->y,5,5,d->color,alpha/7,false,true);
        shape(d->x,d->y,2,2,d->color,alpha,false,true);
        if(d->kind==2)shape(d->x,d->y-5,1,5,d->color,alpha/2,false,true);
    }
    border(status,(int)(now/1000000));
    (void)last_spawn;(void)last_quota;
}
void effects_extent(int band,int *left,int *right){
    int y=band*BAND_ROWS;
    for(int i=0;i<shape_count;i++){
        shape_t s=shapes[i];if(!s.dirty||s.y+s.ry<y||s.y-s.ry>=y+BAND_ROWS)continue;
        if(s.x-s.rx<*left)*left=s.x-s.rx;
        if(s.x+s.rx+1>*right)*right=s.x+s.rx+1;
    }
}
static uint16_t mix(uint16_t dst,uint16_t src,int a){
    unsigned d=(dst>>8)|(dst<<8);d&=65535;
    unsigned r=(((d>>11)&31)*(255-a)+((src>>11)&31)*a)/255;
    unsigned g=(((d>>5)&63)*(255-a)+((src>>5)&63)*a)/255;
    unsigned b=((d&31)*(255-a)+(src&31)*a)/255;
    unsigned v=(r<<11)|(g<<5)|b;return (v>>8)|(v<<8);
}
void effects_draw_band(uint16_t *buf,int y0,int left,int width){
    for(int i=0;i<shape_count;i++){
        shape_t s=shapes[i];
        int ya=s.y-s.ry;if(ya<y0)ya=y0;int yb=s.y+s.ry;if(yb>=y0+BAND_ROWS)yb=y0+BAND_ROWS-1;
        for(int y=ya;y<=yb;y++){
            float fy=(y-s.y)/(float)s.ry;
            int span=(int)(s.rx*sqrtf(fmaxf(0,1-fy*fy)));
            int xa=s.x-span,xb=s.x+span;
            if(xa<left)xa=left;
            if(xb>=left+width)xb=left+width-1;
            for(int x=xa;x<=xb;x++){
                if(s.ring&&abs(x-s.x)<span-2&&abs(y-s.y)<s.ry-1)continue;
                // Preserve a conservative rounded-corner inset for decorations.
                if(x<8||x>471||y<8||y>471)continue;
                uint16_t *pixel=&buf[(y-y0)*width+x-left];*pixel=mix(*pixel,s.color,s.alpha);
            }
        }
    }
}
