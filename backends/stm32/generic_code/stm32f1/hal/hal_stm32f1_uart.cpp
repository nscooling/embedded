#include <stm32f10x_usart.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <misc.h>

#include <hal/stm32fx/usart_conf.hpp>
#include <UartInterface.hpp>
#include <hal/Uart.hpp>

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define STM32F1XX_UART_MAX_COUNT								5

namespace stm32f1xx {

	namespace  uart {

		static ::uart::UartInterface*		gpUarts[STM32F1XX_UART_MAX_COUNT];

		Uart::Uart(unsigned int id, ::uart::UartInterface::uart_callback callback, ::uart::Configuration config)
		{

			//assert params
			assert(id > 0);
			assert(id <= sizeof(gpUarts)/sizeof(::uart::UartInterface*));

			// here perform the initialisation
			USART_InitTypeDef		USART_InitStruct; // this is for the USART2 initilization
			NVIC_InitTypeDef		NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)

			//save callback
			_callback = callback;

			// function used to enable the uart clock
			void 		(*pfvRCC_APBxPeriphClockCmd)(uint32_t, FunctionalState);
			uint32_t	  RCC_APBxPeriph_USARTx;


			// the gpio used
			GPIO_TypeDef* 	GPIOx;
			uint32_t	  	RCC_AHBxPeriph_GPIOx;
			uint32_t		GPIO_Pin_Rx, GPIO_Pin_Tx;			// the selected pin

			gpUarts[id - 1] = this;

			  /* enable APB1 peripheral clock for USART2
			   * note that only USART1 and USART6 are connected to APB2
			   * the other USARTs are connected to APB1
			   */
			   switch (id)
			   {
					case 1: 	pfvRCC_APBxPeriphClockCmd 	= RCC_APB2PeriphClockCmd;
								RCC_APBxPeriph_USARTx 		= RCC_APB2Periph_USART1;
								_USARTx						= USART1;			
								GPIOx						= GPIOA;
								GPIO_Pin_Tx					= GPIO_Pin_9;
								GPIO_Pin_Rx					= GPIO_Pin_10;
								RCC_AHBxPeriph_GPIOx		= RCC_APB2Periph_GPIOA;

								NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;

								break;

					case 2:		pfvRCC_APBxPeriphClockCmd	= RCC_APB1PeriphClockCmd;
								RCC_APBxPeriph_USARTx		= RCC_APB1Periph_USART2;
								_USARTx						= USART2;
								GPIOx						= GPIOA;
								GPIO_Pin_Tx					= GPIO_Pin_2;
								GPIO_Pin_Rx					= GPIO_Pin_3;
								RCC_AHBxPeriph_GPIOx		= RCC_APB2Periph_GPIOA;

								NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;

								break;

					case 3: 	pfvRCC_APBxPeriphClockCmd	= RCC_APB1PeriphClockCmd;
								RCC_APBxPeriph_USARTx		= RCC_APB1Periph_USART3;
								_USARTx						= USART3;
								GPIOx						= GPIOB;
								GPIO_Pin_Tx					= GPIO_Pin_10;
								GPIO_Pin_Rx					= GPIO_Pin_11;
								RCC_AHBxPeriph_GPIOx		= RCC_APB2Periph_GPIOB;

								NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;

								break;

					default:	fprintf(stderr, "Unimplemented uart %i !\n", id);
								abort();
								break;
			} //switch uart id

			pfvRCC_APBxPeriphClockCmd(RCC_APBxPeriph_USARTx, ENABLE);
			RCC_APB2PeriphClockCmd(RCC_AHBxPeriph_GPIOx, ENABLE);

			GPIO_InitTypeDef GPIO_InitStructure;
			
			/* TX Pin */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_Tx;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOx, &GPIO_InitStructure);

			/* RX Pin */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_Rx;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOx, &GPIO_InitStructure);



			/* Now the USART_InitStruct is used to define the
			* properties of USART1
			*/
			USART_InitStruct.USART_BaudRate = ::stm32fx::uart::conf_2_baudrate(config.baudRate);       // the baudrate is set to the value we passed into this init function
			USART_InitStruct.USART_WordLength = ::stm32fx::uart::conf_2_WordLength(config.WordLength);// we want the data frame size to be 8 bits (standard)
			USART_InitStruct.USART_StopBits = ::stm32fx::uart::conf_2_StopBits(config.StopBits);   // we want 1 stop bit (standard)
			USART_InitStruct.USART_Parity = ::stm32fx::uart::conf_2_Parity(config.Parity);    // we don't want a parity bit (standard)
			USART_InitStruct.USART_HardwareFlowControl = ::stm32fx::uart::conf_2_HardwareFlowControl(config.HardwareFlowControl); // we don't want flow control (standard)
			USART_InitStruct.USART_Mode = ::stm32fx::uart::conf_2_Mode(config.Mode); // we want to enable the transmitter and the receiver
			USART_Init(_USARTx, &USART_InitStruct);          // again all the properties are passed to the USART_Init function which takes care of all the bit setting


			USART_ITConfig(_USARTx, USART_IT_RXNE, ENABLE);

			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART2 interrupts
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;     // this sets the subpriority inside the group
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      // the USART2 interrupts are globally enabled
			NVIC_Init(&NVIC_InitStructure);              // the properties are passed to the NVIC_Init function which takes care of the low level stuff

			// finally this enables the complete USART1 peripheral
			USART_Cmd(_USARTx, ENABLE);

		}


		void Uart::send(const char* const str)
		{
			send((const unsigned char*)str, strlen(str));
		}

		void Uart::send(const types::buffer& buf)
		{
			for (auto c : buf)
			{
					while ((_USARTx->SR & USART_FLAG_TC) == (uint16_t) RESET) {}
					_USARTx->DR = (c & (uint16_t) 0x01FF);
			}

		}

		void Uart::send(const unsigned char* buf, unsigned int length)
		{
			for (unsigned int i = 0; i < length; i++)
			{
					while ((_USARTx->SR & USART_FLAG_TC) == (uint16_t) RESET) {}
					_USARTx->DR = (buf[i] & (uint16_t) 0x01FF);
			}
		}

		#define USART_IRQ_CALLBACK(UsartX)	void USART##UsartX##_IRQHandler()									\
											{																	\
												gpUarts[UsartX - 1]->_callback(USART##UsartX ->DR);					\
											}

		extern "C"
		{

			USART_IRQ_CALLBACK(1)
			USART_IRQ_CALLBACK(2)
			USART_IRQ_CALLBACK(3)

		}

	} /* namespace uart  */

} /* namespace stm32f1xx  */


namespace uart
{

	typedef ::stm32f1xx::uart::Uart ConcreteUart;
} //namespace uart