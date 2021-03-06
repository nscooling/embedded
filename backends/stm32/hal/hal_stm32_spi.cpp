#include "hal_stm32.hpp"

#ifndef STM32F0XX
#warning "SPI is only supported for STM32F0"
#else

#include "hal_stm32_spi.hpp"

#include <spi_conf.hpp>
#include <SpiInterface.hpp>

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define STM32F0XX_SPI_MAX_COUNT								5

namespace stm32 {

	namespace  spi {

		static ::spi::SpiInterface*		gpSpis[STM32F0XX_SPI_MAX_COUNT];

		Spi::Spi(unsigned int id, ::spi::SpiInterface::spi_callback_T callback, ::spi::Configuration config)
		{

			//assert params
			assert(id > 0);
			assert(id <= sizeof(gpSpis)/sizeof(::spi::SpiInterface*));

			// here perform the initialisation

			SPI_InitTypeDef			SPI_InitType;
			NVIC_InitTypeDef		NVIC_InitType; // this is used to configure the NVIC (nested vector interrupt controller)

			//save callback
			_callback = callback;

			// function used to enable the spi clock
			void 					(*pfvRCC_SPI_PeriphClockCmd)(uint32_t, FunctionalState);
			void 					(*pfvRCC_UART_PeriphClockCmd)(uint32_t, FunctionalState);
			GPIO_TypeDef*			GPIOId;
			uint32_t				RCC_Periph_SPIx;
			IRQn_Type				IRQn;


			// the gpio used
			GPIO_InitTypeDef 		GPIO_SPI;
			uint32_t	  			RCC_Periph_GPIOx;
			uint32_t				GPIO_Pin_Mosi, GPIO_Pin_Miso, GPIO_Pin_Sck, GPIO_Pin_Nss;			// the selected pin
			uint32_t				GPIO_PinSource_Mosi, GPIO_PinSource_Miso, GPIO_PinSource_Sck, GPIO_PinSource_Nss;			// the selected pin
			uint8_t					Gpio_AlternateFunction_Mosi,Gpio_AlternateFunction_Miso,Gpio_AlternateFunction_Sck,Gpio_AlternateFunction_Nss;

			//memset
			SPI_StructInit(&SPI_InitType);;
			GPIO_StructInit(&GPIO_SPI);


			gpSpis[id - 1] = this;

			switch (id)
			{
				case 1: 	
					_SPIId = SPI1;
					GPIOId = GPIOA;
					pfvRCC_SPI_PeriphClockCmd = RCC_APB2PeriphClockCmd; //SPI1
					RCC_Periph_SPIx = RCC_APB2Periph_SPI1;
					
					pfvRCC_UART_PeriphClockCmd = RCC_AHBPeriphClockCmd; //GPIOA
					RCC_Periph_GPIOx = RCC_AHBPeriph_GPIOA;
							
					/* MOSI Pin */
					GPIO_Pin_Mosi = GPIO_Pin_7;
					
					/* MISO Pin */
					GPIO_Pin_Miso = GPIO_Pin_6;
								
					/* SCK Pin */
					GPIO_Pin_Sck = GPIO_Pin_5;
										
					/* NSS Pin */
					GPIO_Pin_Nss = GPIO_Pin_4;

					/* ALternate configuration for MOSI Pin */
					GPIO_PinSource_Mosi = GPIO_PinSource7;
					Gpio_AlternateFunction_Mosi = GPIO_AF_0;

					/* ALternate configuration for MISO Pin */
					GPIO_PinSource_Miso = GPIO_PinSource6;
					Gpio_AlternateFunction_Miso = GPIO_AF_0;

					/* ALternate configuration for SCK Pin */
					GPIO_PinSource_Sck = GPIO_PinSource5;
					Gpio_AlternateFunction_Sck = GPIO_AF_0;
					
					/* ALternate configuration for NSS Pin */
					GPIO_PinSource_Nss = GPIO_PinSource4;
					Gpio_AlternateFunction_Nss = GPIO_AF_0;

					IRQn = SPI1_IRQn;
					
					break;

				case 2: 	
					_SPIId = SPI2;
					GPIOId = GPIOB;
					pfvRCC_SPI_PeriphClockCmd = RCC_APB1PeriphClockCmd; //SPI2
					RCC_Periph_SPIx = RCC_APB1Periph_SPI2;
					
					pfvRCC_UART_PeriphClockCmd = RCC_AHBPeriphClockCmd; //GPIOB
					RCC_Periph_GPIOx = RCC_AHBPeriph_GPIOB;
							
					/* MOSI Pin */
					GPIO_Pin_Mosi = GPIO_Pin_15;
					
					/* MISO Pin */
					GPIO_Pin_Miso = GPIO_Pin_14;
								
					/* SCK Pin */
					GPIO_Pin_Sck = GPIO_Pin_13;
										
					/* NSS Pin */
					GPIO_Pin_Nss = GPIO_Pin_12;

					/* ALternate configuration for MOSI Pin */
					GPIO_PinSource_Mosi = GPIO_PinSource15;
					Gpio_AlternateFunction_Mosi = GPIO_AF_0;

					/* ALternate configuration for MISO Pin */
					GPIO_PinSource_Miso = GPIO_PinSource14;
					Gpio_AlternateFunction_Miso = GPIO_AF_0;

					/* ALternate configuration for SCK Pin */
					GPIO_PinSource_Sck = GPIO_PinSource13;
					Gpio_AlternateFunction_Sck = GPIO_AF_0;
					
					/* ALternate configuration for NSS Pin */
					GPIO_PinSource_Nss = GPIO_PinSource12;
					Gpio_AlternateFunction_Nss = GPIO_AF_0;

					IRQn = SPI2_IRQn;
					
					break;
					
				default:	
					fprintf(stderr, "Unimplemented spi %i !\n", id);
					abort();
					break;
			} //switch spi id


			//enable clocks
			pfvRCC_SPI_PeriphClockCmd(RCC_Periph_SPIx, ENABLE);
			pfvRCC_UART_PeriphClockCmd(RCC_Periph_GPIOx, ENABLE);

			//configure pin alternate functions
			GPIO_PinAFConfig(GPIOId, GPIO_PinSource_Mosi, Gpio_AlternateFunction_Mosi);
			GPIO_PinAFConfig(GPIOId, GPIO_PinSource_Miso, Gpio_AlternateFunction_Miso);
			GPIO_PinAFConfig(GPIOId, GPIO_PinSource_Sck, Gpio_AlternateFunction_Sck);
			if (config.slaveSelectManagement == ::spi::Configuration::Hard)
			{
				GPIO_PinAFConfig(GPIOId, GPIO_PinSource_Nss, Gpio_AlternateFunction_Nss);
			}

			//configure GPIOs
			GPIO_StructInit(&GPIO_SPI);
			GPIO_SPI.GPIO_Mode = GPIO_Mode_AF;

			/* MOSI Pin */
			GPIO_SPI.GPIO_Pin = GPIO_Pin_Mosi;
			GPIO_Init(GPIOId, &GPIO_SPI);

			/* MISO Pin */
			GPIO_SPI.GPIO_Pin = GPIO_Pin_Miso;
			GPIO_Init(GPIOId, &GPIO_SPI);
			
			/* SCK Pin */
			GPIO_SPI.GPIO_Pin = GPIO_Pin_Sck;
			GPIO_Init(GPIOId, &GPIO_SPI);
			
			/* NSS Pin */
			if (config.slaveSelectManagement == ::spi::Configuration::Hard)
			{
				GPIO_SPI.GPIO_Pin = GPIO_Pin_Nss;
				GPIO_Init(GPIOId, &GPIO_SPI);
			}
	
			//configure SPI
			SPI_InitType.SPI_Direction = ::stm32fx::spi::conf_2_direction(config.direction);
			SPI_InitType.SPI_Mode = ::stm32fx::spi::conf_2_mode(config.mode);
			SPI_InitType.SPI_DataSize = SPI_DataSize_8b;
			SPI_InitType.SPI_CPOL = ::stm32fx::spi::conf_2_ClockPolarity(config.clockPolarity);
			SPI_InitType.SPI_CPHA = ::stm32fx::spi::conf_2_ClockPhase(config.clockPhase);
			SPI_InitType.SPI_NSS = ::stm32fx::spi::conf_2_SlaveSelectManagement(config.slaveSelectManagement);
			SPI_InitType.SPI_BaudRatePrescaler = ::stm32fx::spi::conf_2_BaudRatePrescaler(config.baudRatePrescaler);
			SPI_InitType.SPI_FirstBit = ::stm32fx::spi::conf_2_FirstBitTransmission(config.firstBitTransmission);
			SPI_InitType.SPI_CRCPolynomial = 7;

			SPI_Init(_SPIId, &SPI_InitType);

			//enable RX interrupt on first byte received !
			SPI_RxFIFOThresholdConfig(_SPIId, SPI_RxFIFOThreshold_QF);

			if (config.mode == ::spi::Configuration::Slave)
			{
				/*************** SPI INTERRUPT ****************/
				NVIC_InitType.NVIC_IRQChannel = IRQn;
				NVIC_InitType.NVIC_IRQChannelPriority = 0;
				NVIC_InitType.NVIC_IRQChannelCmd = ENABLE;

				NVIC_Init(&NVIC_InitType);
				
				SPI_I2S_ITConfig(_SPIId, SPI_I2S_IT_RXNE, ENABLE);
			}

			//enable !
			SPI_Cmd(_SPIId, ENABLE);

		}


		void Spi::send(const types::buffer& buf)
		{
			for (auto c : buf)
			{
					send(c);
			}

		}

		void Spi::send(const unsigned char* buf, unsigned int length)
		{
			unsigned int						i;
			
			for (i = 0; i < length; i++)
			{
				send(buf[i]);
			}
		}
		
		uint8_t Spi::send(uint8_t data)
		{
			while (SPI_I2S_GetFlagStatus(_SPIId, SPI_I2S_FLAG_TXE) == RESET);
			SPI_SendData8(_SPIId, data);
			return SPI_ReceiveData8(_SPIId);
		}
		
		uint8_t Spi::recv()
		{
			return SPI_ReceiveData8(_SPIId);
		}

		#define SPI_IRQ_CALLBACK(SpiX)	void SPI##SpiX##_IRQHandler()									\
											{																	\
												gpSpis[SpiX - 1]->_callback(SPI_ReceiveData8(SPI##SpiX));					\
											}

		extern "C"
		{

			SPI_IRQ_CALLBACK(1)
			SPI_IRQ_CALLBACK(2)
		}

	} /* namespace spi  */

} /* namespace stm32  */

#endif //STM32F0XX
