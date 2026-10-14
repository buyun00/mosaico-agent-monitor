#define MIKU_WIDTH 168
#define MIKU_HEIGHT 152
#define MIKU_ASSET_BYTES 5519286u
#define MIKU_ASSET_CRC 0xc62ae293u
typedef struct {unsigned int offset,size;unsigned char delta;} miku_block_t;
static const miku_block_t miku_blocks[]={
{0,26892,1},
{26892,33201,1},
{60093,28171,1},
{88264,30453,1},
{118717,30520,1},
{149237,32743,0},
{181980,33327,0},
{215307,33108,0},
{248415,28763,1},
{277178,33291,0},
{310469,30775,1},
{341244,29281,1},
{370525,31629,1},
{402154,31945,1},
{434099,32092,0},
{466191,29024,0},
{495215,28800,0},
{524015,29975,0},
{553990,31543,0},
{585533,31767,1},
{617300,30625,1},
{647925,31716,1},
{679641,31765,0},
{711406,30134,1},
{741540,28240,1},
{769780,31693,0},
{801473,28245,1},
{829718,31266,0},
{860984,28047,1},
{889031,32071,0},
{921102,32378,1},
{953480,34226,0},
{987706,31758,1},
{1019464,33490,1},
{1052954,34817,0},
{1087771,35477,0},
{1123248,35308,0},
{1158556,35991,0},
{1194547,35112,0},
{1229659,35066,0},
{1264725,34582,0},
{1299307,30510,1},
{1329817,34395,0},
{1364212,28867,1},
{1393079,33625,0},
{1426704,32370,0},
{1459074,32424,0},
{1491498,32509,0},
{1524007,33399,0},
{1557406,35155,0},
{1592561,29268,1},
{1621829,30248,1},
{1652077,28446,1},
{1680523,32486,1},
{1713009,32838,0},
{1745847,33114,1},
{1778961,33663,1},
{1812624,31714,1},
{1844338,31215,1},
{1875553,34093,0},
{1909646,28071,1},
{1937717,32019,0},
{1969736,31498,1},
{2001234,32220,0},
{2033454,32342,0},
{2065796,32433,0},
{2098229,32715,0},
{2130944,31538,1},
{2162482,30442,1},
{2192924,31639,0},
{2224563,31820,0},
{2256383,31937,0},
{2288320,31609,0},
{2319929,29341,1},
{2349270,31957,0},
{2381227,31212,1},
{2412439,30204,1},
{2442643,31663,0},
{2474306,28691,1},
{2502997,29519,1},
{2532516,29433,1},
{2561949,29814,0},
{2591763,30747,0},
{2622510,27850,1},
{2650360,30531,0},
{2680891,29554,1},
{2710445,30842,0},
{2741287,30312,0},
{2771599,30582,0},
{2802181,31161,0},
{2833342,35467,1},
{2868809,33997,1},
{2902806,35620,0},
{2938426,35367,0},
{2973793,34882,0},
{3008675,33484,1},
{3042159,24306,1},
{3066465,34921,1},
{3101386,31404,1},
{3132790,35382,0},
{3168172,34699,0},
{3202871,32030,1},
{3234901,30818,1},
{3265719,34961,0},
{3300680,35322,0},
{3336002,35616,0},
{3371618,35441,1},
{3407059,35081,1},
{3442140,36025,0},
{3478165,35187,1},
{3513352,32849,1},
{3546201,34810,0},
{3581011,34958,0},
{3615969,33567,1},
{3649536,32961,0},
{3682497,33879,1},
{3716376,26810,1},
{3743186,33614,1},
{3776800,34029,0},
{3810829,35091,0},
{3845920,31293,1},
{3877213,30428,1},
{3907641,27835,1},
{3935476,31385,1},
{3966861,33661,0},
{4000522,26134,1},
{4026656,30859,1},
{4057515,32600,1},
{4090115,29030,1},
{4119145,28305,1},
{4147450,30081,1},
{4177531,26677,1},
{4204208,29071,1},
{4233279,30982,1},
{4264261,33259,0},
{4297520,30111,1},
{4327631,25754,1},
{4353385,28899,1},
{4382284,31403,1},
{4413687,33142,0},
{4446829,30358,1},
{4477187,29610,1},
{4506797,30553,1},
{4537350,31556,1},
{4568906,32652,0},
{4601558,31997,0},
{4633555,26042,1},
{4659597,28934,1},
{4688531,25466,1},
{4713997,32375,0},
{4746372,26374,1},
{4772746,22436,1},
{4795182,26966,1},
{4822148,27788,0},
{4849936,27225,0},
{4877161,25563,0},
{4902724,24717,0},
{4927441,24823,0},
{4952264,26050,0},
{4978314,27558,0},
{5005872,27047,0},
{5032919,27393,0},
{5060312,27234,0},
{5087546,27202,0},
{5114748,26419,0},
{5141167,25701,0},
{5166868,25315,0},
{5192183,25237,0},
{5217420,24879,0},
{5242299,24864,0},
{5267163,25315,0},
{5292478,26133,0},
{5318611,26257,0},
{5344868,23063,1},
{5367931,26272,0},
{5394203,21801,1},
{5416004,25453,1},
{5441457,25448,1},
{5466905,25673,1},
{5492578,26708,0}
};
static const unsigned char miku_rgb[768]={253,253,252,250,250,249,247,246,246,248,243,240,251,242,228,248,236,229,248,227,217,249,219,197,249,224,137,241,237,235,198,239,224,238,218,198,211,213,202,124,225,219,115,214,213,112,209,208,114,205,197,104,217,218,104,209,210,105,206,208,102,206,208,105,205,206,102,205,206,104,204,202,102,203,205,105,203,201,102,202,201,102,201,199,92,217,217,93,209,211,97,206,210,94,206,212,94,205,210,96,205,206,90,206,213,90,206,209,89,205,207,84,205,206,98,202,205,93,203,209,94,202,205,96,201,201,86,202,204,198,177,230,193,170,225,191,169,223,190,166,223,218,185,172,152,184,133,118,187,138,194,152,152,143,143,141,161,112,96,113,103,101,92,197,198,90,184,184,73,185,185,75,154,153,50,143,143,74,103,99,75,78,84,35,110,111,33,85,86,158,58,85,76,66,72,69,72,79,69,60,66,66,67,74,65,57,63,60,64,71,57,62,63,56,58,64,52,52,57,50,41,45,32,63,63,18,61,62,29,47,48,35,40,44,33,38,36,20,39,40,32,33,37,25,32,34,30,30,29,17,31,32,32,26,29,22,26,33,20,25,28,21,25,25,13,25,26,27,19,22,17,18,19,13,15,17,9,13,14,4,14,14,4,5,6,1,1,1};
// Generated asset index is prepended by package-miku-firmware.py.
#ifndef MIKU_HOST_TEST
#include "esp_partition.h"
#include "esp_rom_crc.h"
#include "esp_log.h"
#endif
static const unsigned char *miku_source;
static unsigned char *miku_cache;
static tinfl_decompressor *miku_decoder;
static uint16_t miku_colors[256];
static int miku_cached_block=-1;
static bool miku_ready;
#ifndef MIKU_HOST_TEST
static esp_partition_mmap_handle_t miku_map;
#endif
static void miku_init(void){
#ifdef MIKU_HOST_TEST
    extern const unsigned char *host_miku_assets(void);
    miku_source=host_miku_assets();
#else
    const esp_partition_t *p=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,0x40,"miku_assets");
    if(!p||p->size<MIKU_ASSET_BYTES){ESP_LOGE("miku","animation partition missing/too small");return;}
    const void *mapped=NULL;
    if(esp_partition_mmap(p,0,MIKU_ASSET_BYTES,ESP_PARTITION_MMAP_DATA,&mapped,&miku_map)!=ESP_OK){ESP_LOGE("miku","animation map failed");return;}
    miku_source=mapped;
    if(esp_rom_crc32_le(0,miku_source,MIKU_ASSET_BYTES)!=MIKU_ASSET_CRC){ESP_LOGE("miku","animation CRC mismatch; retain dashboard without mascot");esp_partition_munmap(miku_map);miku_source=NULL;return;}
#endif
    miku_cache=heap_caps_malloc(MIKU_WIDTH*MIKU_HEIGHT*5,MALLOC_CAP_SPIRAM);
    miku_decoder=heap_caps_calloc(1,sizeof(*miku_decoder),MALLOC_CAP_SPIRAM);
    if(!miku_cache||!miku_decoder){free(miku_cache);free(miku_decoder);miku_cache=NULL;miku_decoder=NULL;return;}
    for(int i=0;i<256;i++)miku_colors[i]=color(miku_rgb[i*3],miku_rgb[i*3+1],miku_rgb[i*3+2]);
    miku_ready=true;
}
static bool miku_decode(int block){
    if(!miku_ready||block<0||block>=180)return false;
    if(block==miku_cached_block)return true;
    const miku_block_t *b=miku_blocks+block;
    tinfl_init(miku_decoder);size_t in=b->size,out=MIKU_WIDTH*MIKU_HEIGHT*5;
    if(tinfl_decompress(miku_decoder,miku_source+b->offset,&in,miku_cache,miku_cache,&out,TINFL_FLAG_PARSE_ZLIB_HEADER|TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)!=TINFL_STATUS_DONE||out!=MIKU_WIDTH*MIKU_HEIGHT*5){miku_ready=false;return false;}
    if(b->delta)for(int i=MIKU_WIDTH*MIKU_HEIGHT;i<MIKU_WIDTH*MIKU_HEIGHT*5;i++)miku_cache[i]^=miku_cache[i-MIKU_WIDTH*MIKU_HEIGHT];
    miku_cached_block=block;return true;
}
static int miku_action_for_state(int state){
    // idle, attention, done, failed, paused/offline, coding, thinking.
    static const unsigned char actions[]={3,2,5,1,4,0,1};
    return state>=0&&state<7?actions[state]:3;
}
static int miku_frame_at(int action,int64_t elapsed_us){
    if(elapsed_us<0)elapsed_us=0;
    int64_t frame=elapsed_us*30/1000000;
    // Complete once and hold the settled pose. Other gestures loop at five seconds.
    return action==5&&frame>=150?149:(int)(frame%150);
}
static void draw_miku(int state,float unused_seconds){
    (void)unused_seconds;
    static int previous_state=-1;static int64_t started;
    int64_t now=esp_timer_get_time();
    if(state!=previous_state){previous_state=state;started=now;}
    int action=miku_action_for_state(state),frame=miku_frame_at(action,now-started);
    if(!miku_decode(action*30+frame/5))return;
    const unsigned char *src=miku_cache+(frame%5)*MIKU_WIDTH*MIKU_HEIGHT;
    // Precomposited background and a RGB565 lookup avoid per-pixel alpha/color
    // arithmetic in the 30 FPS hot path. Task transitions still fade as a unit.
    int x0=240-MIKU_WIDTH/2+(int)content_dx,y0=224;
    if(content_alpha==255&&x0>=0&&x0+MIKU_WIDTH<=480){
        for(int y=0;y<MIKU_HEIGHT;y++)for(int x=0;x<MIKU_WIDTH;x++)canvas[(y0+y)*480+x0+x]=miku_colors[src[y*MIKU_WIDTH+x]];
    }else for(int y=0;y<MIKU_HEIGHT;y++)for(int x=0;x<MIKU_WIDTH;x++)pixel(x0+x,y0+y,miku_colors[src[y*MIKU_WIDTH+x]],255);
}
