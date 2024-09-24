#include "gd32f20x.h"
#include "systick.h"
#include "usart.h"
#include "stdio.h"
#include "iap.h"
#include "flash.h"
#include "led.h"
/**
 * @brief   main function
 *
 * @note    This function is the entry of the program. It configures the system,
 *          initializes the peripherals and then enters the super loop.
 *
 * @param   None
 *
 * @retval  None
 */
int main(void)
{

	uint32_t RxDataCount = 0;	// 串口接收到的数据计数(旧)
	uint32_t AppCodeLength = 0; // 接收到的app代码长度
	uint32_t RxDataLength = 0;
	uint8_t RxCmdFlag = 0;
	uint8_t AppRunFlag = 0; // 应用程序运行标志,APP已经写入到flash，等待跳转
	/* configure systick */
	systick_config();
	usart_config();
	led_gpio_init();
	while (1)
	{
		gpio_bit_set(GPIOC, GPIO_PIN_14);
		// gpio_bit_reset(GPIOD, GPIO_PIN_2);
		delay_ms(500);
		gpio_bit_reset(GPIOC, GPIO_PIN_14);
		// gpio_bit_set(GPIOD, GPIO_PIN_2);
		delay_ms(500);
		if (USART_RX_CNT) // 接收APP程序放到BUF中
		{
			printf("USART_RX_CNT = %d \r\n", USART_RX_CNT);
			if (RxDataCount == USART_RX_CNT) // 串口没有再收到新数据
			{
				RxDataLength = USART_RX_CNT;
				if (RxCmdFlag == 0 && RxDataLength == 5) // 接收IAP指令
				{
					if (USART_RX_BUF[0] == 0xAA && USART_RX_BUF[1] == 0xAA && USART_RX_BUF[2] == 0xAA && USART_RX_BUF[3] == 0xAA && USART_RX_BUF[4] == 0xAA)
					{
						RxCmdFlag = 1;	  // 接收到更新APP代码指令，标志位置位
						RxDataLength = 0; // 清空指令长度，防止影响后面计算APP代码大小
						printf("Ready to recieve app code,please add a binary file!\r\n");
						// 准备好接收bin文件，等待用户添加
					}
					else if (USART_RX_BUF[0] == 0XAA && USART_RX_BUF[1] == 0XAA && USART_RX_BUF[2] == 0XAA &&
							 USART_RX_BUF[3] == 0XAA && USART_RX_BUF[4] == 0XBB)
					{
						AppRunFlag = 0;
						printf("Before finish IAP\r\n");
						iap_load_app(FLASH_APP1_ADDR); // 执行FLASH APP代码
					}
					else
					{
						CodeUpdateFlag = 0;
						AppCodeLength = 0;
						printf("Command Error!\r\n"); // 未接收到IAP更新指令，其他任何串口发送数据都认为指令错误
					}
				}
				else if (RxCmdFlag == 1 && RxDataLength > 10) // 接收IAP程序
				{
					CodeUpdateFlag = 1; // 代码更新标志位置位，用于应用程序代码接收完成后写FLASH
					RxCmdFlag = 0;
					AppCodeLength = USART_RX_CNT;
					printf("App code recieve complete!\r\n");
					printf("Code size:%dBytes\r\n", AppCodeLength);
				}
				else// 如果代码大小不足10Bytes，认为没有正确添加bin文件
				{
					RxDataLength = 0;
					printf("Not correct file or command!\r\n"); 
				}
				RxDataCount = 0;
				USART_RX_CNT = 0;
			}
			else
			{
				RxDataCount = USART_RX_CNT;
			}
		}
		if (CodeUpdateFlag) // 代码更新标志位置位
		{
			CodeUpdateFlag = 0;
			if (AppCodeLength)
			{
				printf("Updating app code...\r\n");
				if (((*(volatile uint32_t *)(0X20001000 + 4)) & 0xFF000000) == 0x08000000) // 判断代码合法性
				{
					iap_write_appbin(FLASH_APP1_ADDR, USART_RX_BUF, AppCodeLength); // 新代码写入FLASH
					printf("Code Update Complete!Jump to App Code In 5 second!\r\n");
					delay_ms(1000);
					printf("Code Update Complete!Jump to App Code In 4 second!\r\n");
					delay_ms(1000);
					printf("Code Update Complete!Jump to App Code In 3 second!\r\n");
					delay_ms(1000);
					printf("Code Update Complete!Jump to App Code In 2 second!\r\n");
					delay_ms(1000);
					printf("Code Update Complete!Jump to App Code In 1 second!\r\n");
					delay_ms(1000);
					AppRunFlag = 1;
				}
				else
				{
					printf("Code update failed!Please check legality of the binary file!\r\n");
				}
			}
			else
			{
				printf("No Code Can Update!\r\n");
			}
		}

		if (AppRunFlag) // App运行标志置位标志写入完成，开始执行APP的代码
		{
			printf("Start running app code!\r\n");
			delay_ms(10);
			if (((*(volatile uint32_t *)(FLASH_APP1_ADDR + 4)) & 0xFF000000) == 0x08000000) // 判断代码合法性
			{
				AppRunFlag = 0;
				iap_load_app(FLASH_APP1_ADDR); // 执行FLASH APP代码
			}
			else
			{
				printf("App code is illegal!\r\n");
			}
		}
	}
}
