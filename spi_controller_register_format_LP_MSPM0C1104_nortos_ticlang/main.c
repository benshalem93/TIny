#include "ti_msp_dl_config.h"
#include "bma530_driver.h"


#define CLOCK_MHZ   24


void delay_ms(float ms)
{
    delay_cycles(ms * CLOCK_MHZ * 1000);
}


int main(void)
{
    SYSCFG_DL_init();

    /* Enable interrupts */
    DL_SYSCTL_disableSleepOnExit();
    NVIC_EnableIRQ(SPI_0_INST_INT_IRQN);

    /* Set LED to indicate start of transfer */
    DL_GPIO_clearPins(GPIO_LEDS_PORT, (GPIO_LEDS_USER_LED_1_PIN ));

    delay_cycles(DEAFULT_DELAY_CYCLES);
    
    BMA530_AccelXYZ acc;

    while(1) {
        BMA530_ReadAccel(&acc, RANGE_8G);
        if(acc.z > 8 && acc.z < 12)
        {
            DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
            delay_ms(100);
        }
    }

    return 0;
}