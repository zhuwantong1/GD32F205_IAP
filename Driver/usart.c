#include "usart.h"
#include "gd32f20x.h"
#include "stdio.h"
uint8_t USART_RX_BUF[USART_REC_LEN] __attribute__((at(0X20001000))); // 接收缓冲,最大USART_REC_LEN个字节,起始地址为0X20001000.

uint16_t USART_RX_STA = 0;  // 接收状态标记
uint32_t USART_RX_CNT = 0;  // 接收的字节数
uint8_t CodeUpdateFlag = 0; // App代码更新标志

void usart_config(void)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART2);

    /* connect port to USART2 Tx */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* connect port to USART2 Rx */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, 115200);
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);
    usart_parity_config(USART2, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART2, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART2, USART_CTS_DISABLE);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);
    // 使能USART接收中断
    usart_interrupt_enable(USART2, USART_INT_RBNE);
    // 配置NVIC中断优先级并使能中断
    nvic_irq_enable(USART2_IRQn, 0, 0);
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART2, (uint8_t)ch);
    while (RESET == usart_flag_get(USART2, USART_FLAG_TBE))
        ;
    return ch;
}

int fgetc(FILE *f)
{
    uint8_t ch = 0;
    ch = usart_data_receive(USART2);
    return ch;
}

// USART中断处理函数
void USART2_IRQHandler(void)
{
    uint8_t Res;
    if (usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE) != RESET)
    {
        Res = usart_data_receive(USART2);
        //  usart_data_transmit(USART2, (uint8_t)Res);

        if (USART_RX_CNT < USART_REC_LEN)
        {
            USART_RX_BUF[USART_RX_CNT] = Res;
            USART_RX_CNT++;
        }
    }
}
