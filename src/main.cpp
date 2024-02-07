#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"

#include "credentials.h"

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

static const char *TAG = "example:take_picture";

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 1,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

static esp_err_t init_camera(void)
{
    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Initializing camera...");

    while(init_camera() != ESP_OK)
    {
        Serial.println("Failed to initialize camera!");
        delay(1000);
    }

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("Connecting to WiFi %s", WIFI_SSID);
    while(WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
    }
    Serial.print("\n");

    Serial.println("Connected!");
}

#define TARGET_IP "192.168.10.113"
#define TARGET_PORT 8909
WiFiClient client;
WiFiUDP udp;
bool connected = false;

void loop()
{
    /*if (!client.connected())
    {
        Serial.printf("Connecting to: %s:%d\n", TARGET_IP, TARGET_PORT);
        connected = false;
        client.connect(TARGET_IP, TARGET_PORT);
    }
    else
    {
        if (!connected)
        {
            Serial.println("Connected to server!");
        }
        connected = true;
    }
    */

    //ESP_LOGI(TAG, "Taking picture...");
    camera_fb_t *pic = esp_camera_fb_get();

    //if (connected)
    {
        int i = 0;
        Serial.printf("Sending %d bytes to server\n", pic->len);
        udp.beginPacket(TARGET_IP, TARGET_PORT);
        udp.write(pic->buf, pic->len);
        udp.endPacket();
        /*
        while (i < pic->len)
        {
            int remaining_bytes = pic->len - i;
            int buf_size;
            if (remaining_bytes >= 1024)
            {
                buf_size = 1024;
            }
            else
            {
                buf_size = remaining_bytes;
            }
            i += buf_size;
        }*/
        Serial.println("Done!");
        //client.write("Hello world!\n");
        //client.flush();
    }

    // use pic->buf to access the image
    //ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
    esp_camera_fb_return(pic);
    
    Serial.println(millis());
    //delay(1000);
}
