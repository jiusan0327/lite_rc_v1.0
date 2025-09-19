#include <stdio.h>
#include "buzz.h"

void buzz_init(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL<<BUZZ_PIN);
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    BUZZ_Clr;
    buzz_on(100); // Initial buzz for 100 ms

}
void buzz_on(uint32_t duration_ms)
{
    BUZZ_Set;
    vTaskDelay(duration_ms / portTICK_PERIOD_MS);
    BUZZ_Clr;
}

void buzz_off(void)
{
    BUZZ_Clr;
}


