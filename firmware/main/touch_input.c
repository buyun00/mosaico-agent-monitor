#include "touch_input.h"
#include "esp_lcd_touch_cst9220.h"
#include "esp_lcd_panel_io.h"
static esp_lcd_touch_handle_t s_touch;
esp_err_t touch_input_init(i2c_master_bus_handle_t bus) {
    esp_lcd_panel_io_i2c_config_t io=ESP_LCD_TOUCH_IO_I2C_CST9220_CONFIG();
    io.transaction_timeout_ms=20;
    esp_lcd_panel_io_handle_t handle=NULL;
    esp_err_t err=esp_lcd_new_panel_io_i2c(bus,&io,&handle);
    if(err!=ESP_OK)return err;
    // Official Mosaico BSP: shared board I2C, GPIO6 INT, no reset, native XY.
    const esp_lcd_touch_config_t cfg={.x_max=479,.y_max=479,.rst_gpio_num=-1,.int_gpio_num=6,
        .levels={.reset=0,.interrupt=0}};
    err=esp_lcd_touch_new_i2c_cst9220(handle,&cfg,&s_touch);
    if(err!=ESP_OK){esp_lcd_panel_io_del(handle);s_touch=NULL;}
    return err;
}
interaction_t touch_input_poll(uint32_t ms) {
    static uint16_t x=240,y=240;
    uint8_t count=0;bool down=false;
    esp_lcd_touch_point_data_t point={0};
    if(s_touch&&esp_lcd_touch_read_data(s_touch)==ESP_OK&&
        esp_lcd_touch_get_data(s_touch,&point,&count,1)==ESP_OK&&count){
        x=point.x;y=point.y;down=true;
    }
    return interaction_update(down,x,y,ms);
}
