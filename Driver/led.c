#include "led.h"

void led_gpio_init(void)
{
		/* enable the LED clock */
    rcu_periph_clock_enable(RCU_GPIOC);  
		/* enable the SPK clock */
	  rcu_periph_clock_enable(RCU_GPIOD);
    /* configure LED GPIO port */
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
		/* configure SPK GPIO port */
	  gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    /* on LED GPIO pin */
    gpio_bit_reset(GPIOC, GPIO_PIN_14);
		/* off SPK GPIO pin */
		gpio_bit_set(GPIOD, GPIO_PIN_2);//设为高，蜂鸣器不响

}

