
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// flask满了回调函数
void dev_flash_over_fun(int state)
{
    dev_restore_file_talbe();
}
// flash测试task
 void flash_task(void *pvParameters)
 {
     char string_val[30];
     dev_easySql_init(dev_flash_over_fun);

     if (dev_easySql_write_num("hello", 4) != -1)
     {
        drv_com0_printf("write num ok\r\n");
     }
    
     if (dev_easySql_write_num("hello1", 8) != -1)
     {
        drv_com0_printf("write num ok\r\n");
     }
     
     if (dev_easySql_write_num("hello2", 9) != -1)
     {
        drv_com0_printf("write num ok\r\n");
     }

    if (dev_easySql_write_num("hello3", 10) != -1)
     {
        drv_com0_printf("write num ok\r\n");
     }

     int temp_num = 0;
     if (dev_dev_easySql_read_num("hello",&temp_num) != -1)
     {
        drv_com0_printf("read num ok:%d\r\n",temp_num);
     }

       if (dev_dev_easySql_read_num("hello1",&temp_num) != -1)
     {
        drv_com0_printf("read num ok:%d\r\n",temp_num);
     }


       if (dev_dev_easySql_read_num("hello2",&temp_num) != -1)
     {
        drv_com0_printf("read num ok:%d\r\n",temp_num);
     }


    if (dev_easySql_write_string("kirito", "what is") != -1)
     {
        drv_com0_printf("write str ok\r\n");
     }
 
    if (dev_easySql_read_string("kirito",&string_val) != -1)
     {
         drv_com0_printf("str:%s", string_val);
     }

   
      drv_com0_printf("what happend \r\n");
     

     if (dev_easySql_write_string("guojin", "laoshiji") != -1)
     {
        drv_com0_printf("write str ok\r\n");
     }
 
    if (dev_easySql_read_string("guojin", &string_val) != -1)
     {
        drv_com0_printf("str:%s", string_val);
     }


 
     while (1)
     {
         vTaskDelay(1000 /portTICK_RATE_MS);
         ;
    }
 }