/*
Author: Angelo Elias Dalzotto (150633@upf.br)
GEPID - Grupo de Pesquisa em Cultura Digital (http://gepid.upf.br/)
Universidade de Passo Fundo (http://www.upf.br/)
*/
/**
 * Pin assignment:
 * - i2c:
 *    GPIO18: SDA
 *    GPIO19: SDL
 * - no need to add external pull-up resistors, driver will enable internal pull-up resistors.
 */


#include <stdio.h>
#include "driver/i2c.h"
#include "freertos/task.h"

#include "max30100.h"

#define I2C_SDA 18
#define I2C_SCL 19
#define I2C_FRQ 100000;
#define I2C_PORT I2C_NUM_0

max30100_config_t max30100;
max30100_data_t result;

esp_err_t i2c_master_init(i2c_port_t i2c_port){
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA;
    conf.scl_io_num = I2C_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FRQ;
    i2c_param_config(i2c_port, &conf);
    return i2c_driver_install(i2c_port, I2C_MODE_MASTER, 0, 0, 0);
}

void get_bpm(void* param) {
    max30100_data_t result;
    while(true) {
        //Update sensor, saving to "result"
        ESP_ERROR_CHECK(max30100_update(&max30100, &result));
        if(result.pulse_detected) {
            printf("BEAT\n");
            printf("BPM: %f | SpO2: %f%%\n", result.heart_bpm, result.spO2);
        }
        //Update rate: 100Hz
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void app_main()
{
    //Init I2C_NUM_0
    ESP_ERROR_CHECK(i2c_master_init(I2C_PORT));
    //Init sensor at I2C_NUM_0
    ESP_ERROR_CHECK(max30100_init( &max30100, I2C_PORT,
                   MAX30100_DEFAULT_OPERATING_MODE,
                   MAX30100_DEFAULT_SAMPLING_RATE,
                   MAX30100_DEFAULT_LED_PULSE_WIDTH,
                   MAX30100_DEFAULT_IR_LED_CURRENT,
                   true, false ));

    //Start test task
    xTaskCreate(get_bpm, "Get BPM", 10000, NULL, 1, NULL);
}