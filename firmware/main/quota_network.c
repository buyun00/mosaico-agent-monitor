#include "quota_network.h"
#include "quota.h"
#include "dashboard.h"
#include "ambience.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdatomic.h>
#include "cJSON.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "quota_net";
static char s_url[192];
static atomic_bool s_connected, s_reboot, s_provisioning;
static atomic_bool s_setup_requested;
void quota_network_request_setup(void) { atomic_store(&s_setup_requested,true); }

// Only private IPv4 HTTP endpoints; redirects are disabled on the client.
static bool private_url(const char *url)
{
    unsigned a,b,c,d,port; int end = 0;
    if (sscanf(url,"http://%u.%u.%u.%u:%u%n",&a,&b,&c,&d,&port,&end) != 5) return false;
    return a <= 255 && b <= 255 && c <= 255 && d <= 255 && port > 0 && port <= 65535 &&
           (a == 10 || (a == 192 && b == 168) || (a == 172 && b >= 16 && b <= 31)) &&
           url[end] == '/' && !strchr(url,'\r') && !strchr(url,'\n') && !strchr(url,' ');
}
static int hex(char c)
{
    if (c >= '0' && c <= '9') return c-'0';
    if (c >= 'a' && c <= 'f') return c-'a'+10;
    if (c >= 'A' && c <= 'F') return c-'A'+10;
    return -1;
}
static bool form_value(const char *form, const char *key, char *out, size_t capacity)
{
    size_t keylen = strlen(key);
    for (const char *p = form; *p;) {
        const char *end = strchr(p,'&');
        if (!end) end = p + strlen(p);
        if ((size_t)(end-p) > keylen && !strncmp(p,key,keylen) && p[keylen] == '=') {
            size_t n = 0;
            for (p += keylen+1; p < end; ++p) {
                unsigned char ch = *p;
                if (ch == '+') ch = ' ';
                else if (ch == '%') {
                    if (end-p < 3 || hex(p[1]) < 0 || hex(p[2]) < 0) return false;
                    ch = (hex(p[1]) << 4) | hex(p[2]); p += 2;
                }
                if (!ch || n+1 >= capacity) return false;
                out[n++] = ch;
            }
            out[n] = 0; return true;
        }
        p = *end ? end+1 : end;
    }
    return false;
}
#include "setup_page.h"
static esp_err_t setup_scan_error(httpd_req_t *req,const char *message)
{
    httpd_resp_set_status(req,"503 Service Unavailable");
    return httpd_resp_sendstr(req,message);
}
static esp_err_t setup_get(httpd_req_t *req)
{
    httpd_resp_set_type(req,"text/html; charset=utf-8");
    httpd_resp_set_hdr(req,"Cache-Control","no-store");
    return httpd_resp_send(req,setup_page,HTTPD_RESP_USE_STRLEN);
}
static esp_err_t setup_scan(httpd_req_t *req)
{
    wifi_scan_config_t config = {.show_hidden=false};
    if (esp_wifi_scan_start(&config,true) != ESP_OK)
        return setup_scan_error(req,"Scan unavailable; retry or enter SSID manually.");
    uint16_t count=24;
    wifi_ap_record_t *records=calloc(count,sizeof(*records));
    if (!records) { esp_wifi_clear_ap_list(); return httpd_resp_send_err(req,HTTPD_500_INTERNAL_SERVER_ERROR,"Memory unavailable"); }
    esp_err_t err=esp_wifi_scan_get_ap_records(&count,records);
    cJSON *result=cJSON_CreateObject();
    cJSON *networks=result?cJSON_AddArrayToObject(result,"networks"):NULL;
    if (!result || !networks || err!=ESP_OK) { free(records); cJSON_Delete(result); return setup_scan_error(req,"Scan unavailable"); }
    for (unsigned i=0;i<count;i++) {
        records[i].ssid[32]=0;
        if (!records[i].ssid[0]) continue;
        bool duplicate=false;
        for (unsigned j=0;j<i;j++) if (!strcmp((char *)records[j].ssid,(char *)records[i].ssid)) { duplicate=true; break; }
        if (duplicate) continue;
        cJSON *ap=cJSON_CreateObject();
        cJSON_AddStringToObject(ap,"ssid",(char *)records[i].ssid);
        cJSON_AddNumberToObject(ap,"rssi",records[i].rssi);
        cJSON_AddBoolToObject(ap,"secure",records[i].authmode!=WIFI_AUTH_OPEN);
        cJSON_AddItemToArray(networks,ap);
    }
    free(records); char *body=cJSON_PrintUnformatted(result); cJSON_Delete(result);
    if (!body) return httpd_resp_send_err(req,HTTPD_500_INTERNAL_SERVER_ERROR,"Memory unavailable");
    httpd_resp_set_type(req,"application/json"); httpd_resp_set_hdr(req,"Cache-Control","no-store");
    err=httpd_resp_send(req,body,HTTPD_RESP_USE_STRLEN); free(body); return err;
}
static esp_err_t setup_save(httpd_req_t *req)
{
    if (req->content_len <= 0 || req->content_len > 1023) return httpd_resp_send_err(req,HTTPD_400_BAD_REQUEST,"Invalid form");
    static char body[6144], ssid[33], password[64], url[192];
    int read = 0;
    while (read < req->content_len) {
        int n = httpd_req_recv(req,body+read,req->content_len-read);
        if (n <= 0) return ESP_FAIL;
        read += n;
    }
    body[read] = 0;
    bool valid = form_value(body,"ssid",ssid,sizeof ssid) && strlen(ssid) > 0 &&
                 form_value(body,"password",password,sizeof password) && (strlen(password) == 0 || strlen(password) >= 8) &&
                 form_value(body,"url",url,sizeof url) && private_url(url);
    memset(body,0,sizeof body);
    if (!valid) { memset(password,0,sizeof password); return httpd_resp_send_err(req,HTTPD_400_BAD_REQUEST,"Check SSID, password (8-63 bytes), and private IPv4 URL with explicit port."); }
    nvs_handle_t store;
    esp_err_t err = nvs_open("fluidquota",NVS_READWRITE,&store);
    if (err == ESP_OK) {
        err = nvs_set_str(store,"ssid",ssid);
        if (err == ESP_OK) err = nvs_set_str(store,"password",password);
        if (err == ESP_OK) err = nvs_set_str(store,"url",url);
        if (err == ESP_OK) err = nvs_set_u8(store,"setup",0);
        if (err == ESP_OK) err = nvs_commit(store);
        nvs_close(store);
    }
    memset(password,0,sizeof password);
    if (err != ESP_OK) return httpd_resp_send_err(req,HTTPD_500_INTERNAL_SERVER_ERROR,"Could not save settings");
    httpd_resp_sendstr(req,"Saved. Device will restart and connect. Rejoin your normal Wi-Fi.");
    atomic_store(&s_reboot,true);
    return ESP_OK;
}
static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg; (void)data;
    if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) atomic_store(&s_connected,true);
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        atomic_store(&s_connected,false);
        quota_accept_line("FBQ1 -1",0);
        if (!atomic_load(&s_provisioning)) esp_wifi_connect();
    }
}
static int json_integer(cJSON *json,const char *key,int fallback,int maximum)
{
    const cJSON *v=cJSON_GetObjectItemCaseSensitive(json,key);
    if(!cJSON_IsNumber(v)||!isfinite(v->valuedouble)||v->valuedouble<0||v->valuedouble>maximum||floor(v->valuedouble)!=v->valuedouble)return fallback;
    return v->valueint;
}
static bool fetch_quota(void)
{
    esp_http_client_config_t cfg = {.url=s_url, .timeout_ms=4000, .disable_auto_redirect=true,
                                   .buffer_size=1024};
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) return false;
    bool ok = false;
    if (esp_http_client_open(client,0) == ESP_OK) {
        int64_t length = esp_http_client_fetch_headers(client);
        if (length >= 0 && length < 6144 && esp_http_client_get_status_code(client) == 200) {
            static char body[6144];
            int n = esp_http_client_read_response(client,body,sizeof body-1);
            if (n > 0 && n < sizeof body-1 && esp_http_client_is_complete_data_received(client)) {
                body[n] = 0;
                cJSON *json = cJSON_Parse(body);
                if (cJSON_IsObject(json)) {
                    dashboard_update(json);
                    int percent=json_integer(json,"weekly_percent",-1,100);
                    char line[16]; snprintf(line,sizeof line,"FBQ1 %d",percent);
                    quota_accept_line(line,(uint32_t)(esp_timer_get_time()/1000000));
                    // Connection and task availability are independent of quota support.
                    ok=true;
                    ambience_view_t activity={
                        .reset_seconds=json_integer(json,"reset_seconds",-1,604800),
                        .running=json_integer(json,"running",0,64),
                        .attention=json_integer(json,"attention",0,64),
                        .failed=json_integer(json,"failed",0,64),
                        .tasks_available=cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(json,"tasks_available")),
                        .event_epoch=(uint32_t)json_integer(json,"event_epoch",0,0x7fffffff),
                        .done_seq=(uint32_t)json_integer(json,"done_seq",0,0x7fffffff),
                        .done_age_seconds=(uint32_t)json_integer(json,"done_age_seconds",0x7fffffff,0x7fffffff)};
                    ambience_update(activity,(uint32_t)(esp_timer_get_time()/1000000));
                }
                cJSON_Delete(json);
            }
        }
    }
    esp_http_client_cleanup(client);
    return ok;
}
static void network_task(void *arg)
{
    (void)arg;
    bool setup = false;
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) { ESP_LOGE(TAG,"NVS unavailable (%s); retained without erase",esp_err_to_name(err)); vTaskDelete(NULL); return; }
    wifi_config_t station = {0};
    bool wifi_started = false;
    char ssid[33] = {0}, password[64] = {0};
    nvs_handle_t store;
    if (nvs_open("fluidquota",NVS_READONLY,&store) == ESP_OK) {
        uint8_t force_setup = 0;
        nvs_get_u8(store,"setup",&force_setup);
        if (force_setup) setup = true;
        size_t ns = sizeof ssid, np = sizeof password, nu = sizeof s_url;
        if (nvs_get_str(store,"ssid",ssid,&ns) != ESP_OK || nvs_get_str(store,"password",password,&np) != ESP_OK ||
            nvs_get_str(store,"url",s_url,&nu) != ESP_OK || !private_url(s_url)) setup = true;
        nvs_close(store);
    } else setup = true;
    if (!ssid[0]) setup = true;
    memcpy(station.sta.ssid,ssid,strlen(ssid));
    memcpy(station.sta.password,password,strlen(password));
    memset(password,0,sizeof password);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,wifi_event,NULL));
    if (!setup) {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&station));
        ESP_ERROR_CHECK(esp_wifi_start());
        wifi_started = true;
        esp_wifi_connect();
        // Wrong credentials or unavailable AP: provide on-device recovery after 30s.
        for (int i=0; i<60 && !atomic_load(&s_connected); ++i) vTaskDelay(pdMS_TO_TICKS(500));
        setup = !atomic_load(&s_connected);
    }
    memset(&station,0,sizeof station);
    httpd_handle_t server = NULL;
    if (setup) {
        atomic_store(&s_provisioning,true);
        if (wifi_started) ESP_ERROR_CHECK(esp_wifi_stop());
        esp_netif_create_default_wifi_ap();
        wifi_config_t ap = {0};
        strcpy((char *)ap.ap.ssid,"FLUIDBOX-SETUP");
        unsigned pin = 10000000u + esp_random()%90000000u;
        snprintf((char *)ap.ap.password,sizeof ap.ap.password,"%08u",pin);
        ap.ap.authmode=WIFI_AUTH_WPA2_PSK; ap.ap.channel=1; ap.ap.max_connection=1;
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP,&ap));
        wifi_config_t empty_station={0};
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&empty_station));
        ESP_ERROR_CHECK(esp_wifi_start());
        quota_set_setup_pin(pin);
        httpd_config_t cfg = HTTPD_DEFAULT_CONFIG(); cfg.stack_size = 6144;
        ESP_ERROR_CHECK(httpd_start(&server,&cfg));
        const httpd_uri_t get = {.uri="/",.method=HTTP_GET,.handler=setup_get};
        const httpd_uri_t post = {.uri="/save",.method=HTTP_POST,.handler=setup_save};
        ESP_ERROR_CHECK(httpd_register_uri_handler(server,&get));
        ESP_ERROR_CHECK(httpd_register_uri_handler(server,&post));
        const httpd_uri_t scan={.uri="/scan",.method=HTTP_GET,.handler=setup_scan};
        ESP_ERROR_CHECK(httpd_register_uri_handler(server,&scan));
        while (!atomic_load(&s_reboot)) vTaskDelay(pdMS_TO_TICKS(500));
        vTaskDelay(pdMS_TO_TICKS(1000)); esp_restart();
    }
    for (;;) {
        if (!atomic_load(&s_connected) || !fetch_quota()) quota_accept_line("FBQ1 -1",0);
        for (int i=0;i<6;++i) {
            if (atomic_load(&s_setup_requested)) {
                if (nvs_open("fluidquota",NVS_READWRITE,&store) == ESP_OK) {
                    if (nvs_set_u8(store,"setup",1) == ESP_OK && nvs_commit(store) == ESP_OK) esp_restart();
                    nvs_close(store);
                }
                atomic_store(&s_setup_requested,false);
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}
void quota_network_start(void)
{
    ESP_ERROR_CHECK(xTaskCreate(network_task,"quota_net",8192,NULL,3,NULL) == pdPASS ? ESP_OK : ESP_ERR_NO_MEM);
}
