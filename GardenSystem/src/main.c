#include <stdio.h>
#include <stdint.h>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <rom/ets_sys.h>
#include "esp_err.h"
#include "bme280.h"
#include "esp_adc/adc_oneshot.h"

#define I2C_MASTER_SCL_IO      9
#define I2C_MASTER_SDA_IO      8
#define I2C_MASTER_NUM         I2C_NUM_0
#define I2C_MASTER_FREQ_HZ     100000

#define SHT31_SENSOR_ADDRESS   0x44
#define BME280_SENSOR_ADDRESS   0x77
#define VEML7700_SENSOR_ADDRESS  0x10

void bme280_test_chip_id(void);
void i2c_init(void);
void adc_init(void);
void sht31_read(void);
void veml7700_read(void);
void bme280_initialize();
void bme280_read(void);
void ml8511_read(void);
BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);
BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);
void bme280_delay_us(uint32_t period, void *intf_ptr);

// bus struct for i2c
typedef struct {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t sht31;
    i2c_master_dev_handle_t bme280;
    i2c_master_dev_handle_t veml7700;
} i2c_system_t;

typedef struct {
    adc_oneshot_unit_handle_t ml8511;
} adc_system_t;



static i2c_system_t i2c;
static adc_system_t adc;

struct bme280_dev bme280_device;
struct bme280_calib_data bme280_calibration_data;
struct bme280_data bme280_comp_data;
struct bme280_settings bme280_device_settings;

void app_main() {
    
    // delay to have time for the output to print to monitor
    vTaskDelay(pdMS_TO_TICKS(2000));
    i2c_init();
    adc_init();
    bme280_initialize();
    printf("garden controller online\n");

    while(true) {
        bme280_test_chip_id();
        sht31_read();
        printf("\n");
        bme280_read();
        printf("\n");
        ml8511_read();
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
        
    /*while (true)
{
    bme280_test_chip_id();
    vTaskDelay(pdMS_TO_TICKS(100));
}
    */
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

    /*
    #########################################################################################
    */

    // configure SHT31 device
    i2c_device_config_t sht31_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SHT31_SENSOR_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    // add SHT31 to bus
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c.bus, &sht31_cfg, &i2c.sht31));

    printf("SHT31 sensor added to i2c bus\n");
    
    // // ~*~*~*~*~*~*~ load bearing read function ~*~*~*~*~*~*~
    //sht31_read();

    /*
    #########################################################################################
    */

    // configure BME280 device
    i2c_device_config_t bme280_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BME280_SENSOR_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    // add BME280 to bus
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c.bus, &bme280_cfg, &i2c.bme280));
    bme280_test_chip_id();
    bme280_test_chip_id();

    printf("BME280 sensor added to i2c bus\n");
   

    /*
    #########################################################################################
    */

    // configure VEML7700 device
    i2c_device_config_t veml7700_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = VEML7700_SENSOR_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    // add VEML7700 to bus
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c.bus, &veml7700_cfg, &i2c.veml7700));

    printf("VEML7700 sensor added to i2c bus\n");

}

void adc_init() {
    adc_oneshot_unit_init_cfg_t ml8511_cfg = {
        .unit_id = ADC_UNIT_1,                  // adc 1
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&ml8511_cfg, &adc.ml8511));

    adc_oneshot_chan_cfg_t ml8511_chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    // set to adc 1 channel 3 (gpio4)
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc.ml8511, ADC_CHANNEL_3, &ml8511_chan_cfg));
    
    printf("ML8511 sensor has been initialized on ADC\n");
}

void ml8511_read() {
    int raw_value;

    ESP_ERROR_CHECK(adc_oneshot_read(adc.ml8511, ADC_CHANNEL_3, &raw_value));

    printf("UV ADC raw value: %d\n", raw_value);
    vTaskDelay(pdMS_TO_TICKS(2000));
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

            printf("SHT31 Temperature: %.3f F (%.3f C)\n", temperature_f, temperature_c);
            printf("SHT31 Humidity: %.3f %% rH\n\n", humidity);
        }

        else {
            printf("Failed to read sensor sht31\n");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));

    }
}

void veml7700_read() {
    /*
    The VEML7700 contains actual six 16 bit command codes for operation control, parameter setup, and result buffering. All
    registers are accessible via I2C communication.
    */

}

void bme280_initialize() {

    /*
    Each compensation word is a 16-bit signed or
    unsigned integer value stored in two’s complement. As the memory is organized into 8-bit words, two
    words must always be combined in order to represent the compensation word. The 8-bit registers are
    named calib00…calib41 and are stored at memory addresses 0x88…0xA1 and 0xE1…0xE7. The
    corresponding compensation words are named dig_T# for temperature compensation related values,
    dig_P# for pressure related values and dig_H# for humidity related value

    0x88 / 0x89         dig_T1 [7:0] / [15:8]   unsigned short
    0x8A / 0x8B         dig_T2 [7:0] / [15:8]   signed short
    0x8C / 0x8D         dig_T3 [7:0] / [15:8]   signed short
    0x8E / 0x8F         dig_P1 [7:0] / [15:8]   unsigned short
    0x90 / 0x91         dig_P2 [7:0] / [15:8]   signed short
    0x92 / 0x93         dig_P3 [7:0] / [15:8]   signed short
    0x94 / 0x95         dig_P4 [7:0] / [15:8]   signed short
    0x96 / 0x97         dig_P5 [7:0] / [15:8]   signed short
    0x98 / 0x99         dig_P6 [7:0] / [15:8]   signed short
    0x9A / 0x9B         dig_P7 [7:0] / [15:8]   signed short
    0x9C / 0x9D         dig_P8 [7:0] / [15:8]   signed short
    0x9E / 0x9F         dig_P9 [7:0] / [15:8]   signed short
    0xA1                dig_H1 [7:0]            unsigned char
    0xE1 / 0xE2         dig_H2 [7:0] / [15:8]   signed short
    0xE3                dig_H3 [7:0]            unsigned char
    0xE4 / 0xE5[3:0]    dig_H4 [11:4] / [3:0]   signed short
    0xE5[7:4] / 0xE6    dig_H5 [3:0] / [11:4]   signed short
    0xE7                dig_H6                  signed char
    */


    /*
    @param[in] reg_addr       : Register address from which data is read.
    @param[out] reg_data      : Pointer to data buffer where read data is stored.
    @param[in] len            : Number of bytes of data to be read.
    @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
                                for interface related call backs.
    */
    /*
    uint8_t reg = 0xF7;
    esp_err_t rslt = i2c_master_transmit(i2c.bme280, &reg, 8, 100);
    if(rslt == ESP_OK) {
        printf("trasmit in initialize worked?\n");
    }
    else {
        printf("trasmit failed\n");
    }
    */
    
    bme280_device.intf = BME280_I2C_INTF;
    bme280_device.intf_ptr = &i2c;
    bme280_device.read = bme280_i2c_read;
    bme280_device.write = bme280_i2c_write;
    bme280_device.delay_us = bme280_delay_us;

    // ~*~*~*~*~*~*~ load bearing test functions ~*~*~*~*~*~*~
    //bme280_test_chip_id();
    //bme280_test_chip_id();
     
    int8_t result = bme280_init(&bme280_device);

    if(result == ESP_OK) {
        printf("bme280 configured\n");

        
    }
    else {
        printf("bme280 failed to configure\n");
        printf("BME280 error code for init: %d\n", result);
    }

    bme280_device_settings.osr_h = BME280_OVERSAMPLING_4X;
    bme280_device_settings.osr_p = BME280_OVERSAMPLING_4X;
    bme280_device_settings.osr_t = BME280_OVERSAMPLING_4X;
    bme280_device_settings.filter = BME280_FILTER_COEFF_OFF;
    bme280_device_settings.standby_time = BME280_STANDBY_TIME_125_MS;

    uint8_t settings_sel = BME280_SEL_OSR_PRESS | BME280_SEL_OSR_TEMP | BME280_SEL_OSR_HUM | BME280_SEL_FILTER;

    result = bme280_set_sensor_settings(settings_sel, &bme280_device_settings, &bme280_device);

    if(result == ESP_OK) {
        printf("BME280 sensor settings set\n");
    }
    else {
        printf("Could not set settings for BME280\n");
    }

    result = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &bme280_device);
    if(result == ESP_OK) {
        printf("BME280 powermode set\n");
    }
    else {
        printf("BME280 powermode NOT set\n");
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    
    
}



void bme280_read() {
    
    /* TODO: move to readme. also not really needed since im using the api. still its nice to know but the compensation math looks ass
    Read register is 0xF7 and continues to 0xFE

    0xF7                pressure MSB
    0xF8                pressure LSB
    0xF9                pressure XLSB
    
    0xFA                temperature MSB
    0xFB                temperature LSB
    0xFC                temperature XLSB

    0xFD                humidity MSB
    0xFE                humidity LSB
    
    */

    
    
    // start register
    //uint8_t reg = 0xF7;

    // trasmit command or something
    esp_err_t result = bme280_get_sensor_data(BME280_TEMP | BME280_PRESS | BME280_HUM, &bme280_comp_data, &bme280_device);
    if(result == ESP_OK) {
        

        printf("bme280 Temperature: %.2f F (%.2f C)\n", (bme280_comp_data.temperature * (9.0f/5.0f) + 32.0f), bme280_comp_data.temperature);
        printf("bme280 Humidity: %.3f %% rH\n", bme280_comp_data.humidity);
        printf("bme280 Pressure: %.2f hPa\n\n", bme280_comp_data.pressure  / 100.0);

    }
    else {
        printf("bme280_get_sensor_data error!! shit didnt work!!\n");
    }


    //struct bme280_uncomp_data uncomp_data;

    // data will come in 8 bytes
    //uint8_t data[8];

    //i2c_master_receive(i2c.bme280, data, 8, 100);

    // raw ADC values. 20 bits for pressure and temperature and 16 for humidity

    // shift by 12 bytes (1.5 bytes), bitwise OR with data[1] shifted 4 bytes (half byte), bitwise OR with data[2] shifted 4 bytes
    //uncomp_data.pressure = (data[0] << 12) | (data[1] << 4) | (data[2] << 4);
    //uncomp_data.temperature = (data[3] << 12) | (data[4] << 4) | (data[5] << 4);

    // shift by 8 bits (1 byte). combine by bitwise OR with data[7]
    //buncomp_data.humidity = (data[6] << 8) | (data[7]);

    //int8_t rslt = bme280_compensate_data(BME280_TEMP | BME280_PRESS | BME280_HUM, &uncomp_data, &bme280_comp_data, &dev.calib_data);
    //if(rslt == BME280_OK) {
    //    float temp_c = bme280_comp_data.temperature / 100.0f;
    //    printf("bme280 temp data does this wprk who knows: %.2f", temp_c);
    //}
    //printf("bme280 temp: ");
    
    //uint8_t rslt = bme280_compensate_data
    vTaskDelay(pdMS_TO_TICKS(2000));
}

// put this bosch bullshit in a separate file
BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr) {
    i2c_system_t *i2c_ptr = (i2c_system_t *)intf_ptr;

    esp_err_t result;

    result = i2c_master_transmit_receive(i2c_ptr->bme280, &reg_addr, 1, data, len, -1);

    

    /*// debug ----------------------------------------------------
    printf("BME280 I2C read: reg=0x%02X len=%lu result=%s\n",
       reg_addr,
       len,
       esp_err_to_name(result));
    */// end debug -------------------------------------------------

    if(result != ESP_OK) {
        return result;
    }
    
    return result;
    

}

BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {

    i2c_system_t *i2c_ptr = (i2c_system_t *)intf_ptr;

    uint8_t buffer[len + 1];

    buffer[0] = reg_addr;

    for(int i = 0; i < len; i++) {
        buffer[i + 1] = data[i];
    }
    esp_err_t result;
    result = i2c_master_transmit(i2c_ptr->bme280, buffer, len + 1, -1);
    if(result == ESP_OK) {
        return result;
    }
    else {
        return result;
    }
    //return i2c_master_transmit(i2c_ptr->bme280, buffer, len + 1, -1);

}

void bme280_delay_us(uint32_t period, void *intf_ptr) {
    ets_delay_us(period);
}
void bme280_test_chip_id()
{
    uint8_t reg = 0xD0;
    uint8_t chip_id;

    esp_err_t result = i2c_master_transmit_receive(
        i2c.bme280,
        &reg,
        1,
        &chip_id,
        1,
        -1
    );

    printf("BME280 chip ID read result: %s\n", esp_err_to_name(result));
    printf("BME280 chip ID: 0x%02X\n", chip_id);
}




