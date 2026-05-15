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



void app_main(void)
{
    /*
        Configure I2C bus
    */
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };



    /*
        Create I2C bus
    */
    i2c_master_bus_handle_t bus_handle;

    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &bus_handle
        )
    );



    /*
        Configure sensor device
    */
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SHT31_SENSOR_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };



    /*
        Add sensor to bus
    */
    i2c_master_dev_handle_t dev_handle;

    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(
            bus_handle,
            &dev_cfg,
            &dev_handle
        )
    );



    printf("SHT31 sensor initialized\n");



    while (1)
    {
        /*
            Command:
            High repeatability measurement
        */
        uint8_t cmd[] = {0x24, 0x00};



        /*
            Send measurement command
        */
        esp_err_t result =
            i2c_master_transmit(
                dev_handle,
                cmd,
                2,
                100
            );



        if (result == ESP_OK)
        {
            /*
                Wait for measurement
            */
            vTaskDelay(pdMS_TO_TICKS(20));



            /*
                Read sensor response
            */
            uint8_t data[6];

            result =
                i2c_master_receive(
                    dev_handle,
                    data,
                    6,
                    100
                );



            if (result == ESP_OK)
            {
                /*
                    Convert raw temperature
                */
                uint16_t raw_temp =
                    (data[0] << 8) | data[1];



                /*
                    Convert raw humidity
                */
                uint16_t raw_humidity =
                    (data[3] << 8) | data[4];



                /*
                    Convert to Celsius
                */
                float temperature =
                    -45.0f +
                    175.0f *
                    ((float)raw_temp / 65535.0f);



                /*
                    Convert to RH %
                */
                float humidity =
                    100.0f *
                    ((float)raw_humidity / 65535.0f);



                printf(
                    "Temperature: %.2f C\n",
                    (temperature * 1.8) + 32
                );



                printf(
                    "Humidity: %.2f %%\n\n",
                    humidity
                );
            }
            else
            {
                printf(
                    "Failed to read sensor\n"
                );
            }
        }
        else
        {
            printf(
                "Failed to send command\n"
            );
        }



        /*
            Wait before next reading
        */
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}