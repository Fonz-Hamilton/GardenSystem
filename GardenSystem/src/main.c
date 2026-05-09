#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {
    while(true) {
        printf("garden controller online\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


