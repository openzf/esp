#include "i2c_app_demo.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "driver/i2c.h"
static const char *TAG = "main";


#define I2C_EXAMPLE_MASTER_SCL_IO           2                /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO           14               /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM              I2C_NUM_0        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0                /*!< I2C master do not need buffer */

#define I2C_DEV_SENSOR_ADDR                 0x68             /*!< slave address for MPU6050 sensor */
#define I2C_DEV_SENSOR_START                   0x41             /*!< Command to set measure mode */
#define I2C_DEV_SENSOR_AM_I                    0x75             /*!< Command to read WHO_AM_I reg */

#define WRITE_BIT                           I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                            I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                       0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                             0x0              /*!< I2C ack value */
#define NACK_VAL                            0x1              /*!< I2C nack value */
#define LAST_NACK_VAL                       0x2              /*!< I2C last_nack value */



// 寄存器地址
#define SMPLRT_DIV      0x19
#define CONFIG          0x1A

// 配置i2c模式
static esp_err_t i2c_example_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = 0;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = 0;
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
    return ESP_OK;
}


static esp_err_t i2c_dev_write(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data, size_t data_len)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // 写目标设备地址, 目标设备地址响应
    i2c_master_write_byte(cmd, I2C_DEV_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    // 写目标设备的寄存器, 目标设备寄存器响应
    i2c_master_write_byte(cmd, reg_address, ACK_CHECK_EN);
    // 写目标设备的 数据, 数据响应
    i2c_master_write(cmd, data, data_len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}


static esp_err_t i2c_dev_read(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data, size_t data_len)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, I2C_DEV_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_address, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        return ret;
    }

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, I2C_DEV_SENSOR_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data, data_len, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}


static void i2c_dev_init(int i2c_num)
{
    uint8_t sensor_data[14];

    uint8_t cmd_data;
    i2c_example_master_init();

    cmd_data = 0x00;    // reset mpu6050
    i2c_dev_write(i2c_num, SMPLRT_DIV, &cmd_data, 1);
    cmd_data = 0x07;    // Set the SMPRT_DIV
    i2c_dev_write(i2c_num,  CONFIG, &cmd_data, 1);

    i2c_dev_read(i2c_num, 0x07, sensor_data, 2);

}

static void i2c_task_example(void *arg)
{

    i2c_dev_init(I2C_EXAMPLE_MASTER_NUM);
    while (1) {

        vTaskDelay(100 / portTICK_RATE_MS);
    }
    i2c_driver_delete(I2C_EXAMPLE_MASTER_NUM);
}


void i2c_app_demo_init()
{
     xTaskCreate(i2c_task_example, "i2c_task_example", 2048, NULL, 10, NULL);
}