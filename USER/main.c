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

	uint32_t RxDataCount = 0;	// ���ڽ��յ������ݼ���(��)
	uint32_t AppCodeLength = 0; // ���յ���app���볤��
	uint32_t RxDataLength = 0;
	uint8_t RxCmdFlag = 0;
	uint8_t AppRunFlag = 0; // Ӧ�ó������б�־,APP�Ѿ�д�뵽flash���ȴ���ת
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
		if (USART_RX_CNT) // ����APP����ŵ�BUF��
		{
			printf("USART_RX_CNT = %d \r\n", USART_RX_CNT);
			if (RxDataCount == USART_RX_CNT) // ����û�����յ�������
			{
				RxDataLength = USART_RX_CNT;
				if (RxCmdFlag == 0 && RxDataLength == 5) // ����IAPָ��
				{
					if (USART_RX_BUF[0] == 0xAA && USART_RX_BUF[1] == 0xAA && USART_RX_BUF[2] == 0xAA && USART_RX_BUF[3] == 0xAA && USART_RX_BUF[4] == 0xAA)
					{
						RxCmdFlag = 1;	  // ���յ�����APP����ָ���־λ��λ
						RxDataLength = 0; // ���ָ��ȣ���ֹӰ��������APP�����С
						printf("Ready to recieve app code,please add a binary file!\r\n");
						// ׼���ý���bin�ļ����ȴ��û����
					}
					else if (USART_RX_BUF[0] == 0XAA && USART_RX_BUF[1] == 0XAA && USART_RX_BUF[2] == 0XAA &&
							 USART_RX_BUF[3] == 0XAA && USART_RX_BUF[4] == 0XBB)
					{
						AppRunFlag = 0;
						printf("Before finish IAP\r\n");
						iap_load_app(FLASH_APP1_ADDR); // ִ��FLASH APP����
					}
					else
					{
						CodeUpdateFlag = 0;
						AppCodeLength = 0;
						printf("Command Error!\r\n"); // δ���յ�IAP����ָ������κδ��ڷ������ݶ���Ϊָ�����
					}
				}
				else if (RxCmdFlag == 1 && RxDataLength > 10) // ����IAP����
				{
					CodeUpdateFlag = 1; // ������±�־λ��λ������Ӧ�ó�����������ɺ�дFLASH
					RxCmdFlag = 0;
					AppCodeLength = USART_RX_CNT;
					printf("App code recieve complete!\r\n");
					printf("Code size:%dBytes\r\n", AppCodeLength);
				}
				else// ��������С����10Bytes����Ϊû����ȷ���bin�ļ�
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
		if (CodeUpdateFlag) // ������±�־λ��λ
		{
			CodeUpdateFlag = 0;
			if (AppCodeLength)
			{
				printf("Updating app code...\r\n");
				if (((*(volatile uint32_t *)(0X20001000 + 4)) & 0xFF000000) == 0x08000000) // �жϴ���Ϸ���
				{
					iap_write_appbin(FLASH_APP1_ADDR, USART_RX_BUF, AppCodeLength); // �´���д��FLASH
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

		if (AppRunFlag) // App���б�־��λ��־д����ɣ���ʼִ��APP�Ĵ���
		{
			printf("Start running app code!\r\n");
			delay_ms(10);
			if (((*(volatile uint32_t *)(FLASH_APP1_ADDR + 4)) & 0xFF000000) == 0x08000000) // �жϴ���Ϸ���
			{
				AppRunFlag = 0;
				iap_load_app(FLASH_APP1_ADDR); // ִ��FLASH APP����
			}
			else
			{
				printf("App code is illegal!\r\n");
			}
		}
	}
}
