#include <stdio.h>
#include <stdint.h>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#define I2C_MASTER_SCL_IO      9
#define I2C_MASTER_SDA_IO      8
#define I2C_MASTER_NUM         I2C_NUM_0
#define I2C_MASTER_FREQ_HZ     100000

#define SHT31_SENSOR_ADDRESS   0x44

void i2c_init(void);
void sht31_read(void);

typedef struct {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t sht31;
} i2c_system_t;

static i2c_system_t i2c;

void app_main() {
    i2c_init();

    while(true) {
        printf("garden controller online\n");
        sht31_read();
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void i2c_init() {

    //I2C master bus configuration
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    // Create master bus

    // terminate the program in case the code is not ESP_OK. Prints the 
    // error code, error location, and the failed statement to serial output.
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c.bus));

    // configure SHT31 device
    i2c_device_config_t sht31_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SHT31_SENSOR_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    // add SHT31 to bus
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c.bus, &sht31_cfg, &i2c.sht31));

    printf("SHT31 sensor initialized\n");

}

void sht31_read() {

    // TODO: move table to readme probably
    /*
                            Single Shot mode
    ________________________________________________________________
    |               Condition               |       Hex. Code       |
    | Repeatability |   Clock Stretching    |   MSB     |   LSB     |
    ________________________________________________________________
    |   High        |                       |           |   06      |
    |   Medium      |       enabled         |   0x2C    |   0D      |
    |   Low         |                       |           |   10      |
    ________________________________________________________________
    |   High        |                       |           |   00      |
    |   Medium      |       disabled        |   0x24    |   0B      |
    |   Low         |                       |           |   16      |
    ________________________________________________________________
   */
    // One byte array
    uint8_t cmd[] = {0x24, 0x00};
    
    // send command. to sht31 address, command, 2 bytes, how long to wait (ms)
    esp_err_t result = i2c_master_transmit(i2c.sht31, cmd, 2, 100);

    if(result == ESP_OK) {
        // wait for measurment
        vTaskDelay(pdMS_TO_TICKS(20));

        // read response

        // response is 6 bytes
        // Byte 0-1: Temperature (MSB, LSB) 
        // Byte 2: CRC (temperature) (checksum)
        // Byte 3-4: humidity (MSB, LSB)
        // Byte 5: CRC (humidity) (checksum)

        uint8_t data[6];

        result = i2c_master_receive(i2c.sht31, data, 6, 100);

        if(result == ESP_OK) {
            // convert raw temp
            // shift high byte left by 8 bits (1 byte). combine with data[1] with bitwise OR
            uint16_t raw_temp = (data[0] << 8 | data[1]);

            // convert raw humidity
            // again, shift high byte left by 1 byte. combine with data[4] with bitwise OR
            // skip data[2] because thats checksum
            uint16_t raw_humidity = (data[3] << 8 | data[4]);

            /*
            Formula
            RH = 100 * (SRH/2^16-1)
            T(C) = -45 + 175 * (ST/(2^16)-1)
            T(F) = -49 + 315 * (ST/(2^16)-1) 
            */

            // convert to F
            float temperature_f = -49.0f + 315.0f * ((float)raw_temp / 65535.0f);

            // convert to C
            float temperature_c = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);

            // convert relative humidity
            float humidity = 100.0f * ((float)raw_humidity / 65535.0f);

            printf("Temperature: %.3fF (%.3fC)\n", temperature_f, temperature_c);
            printf("Humidity: %.3f %%\n\n", humidity);
        }

        else {
            printf("Failed to read sensor");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));

    }
}
