#pragma once

#include <stm32f10x_usart.h>

#include <UartInterface.hpp>

namespace stm32f1xx {

	namespace  uart {

		class Uart : public ::uart::UartInterface
		{
		public:
			// steps to init an uart :
			//	- enable the uart clock
			//	- enable clock for used IO pins
			//	- init the GPIO struct
			//	- init the uart
			//	- configure the alternate function pin
			//	- configure the uart interrupt line
			//	- nvic init
			// enable the complete uart peripheral

			/**
			 * \param id an integer argument for USARTid (id from 1 to STM32F1XX_UART_MAX_COUNT).
			 * \param callback function callback to call when a character has received.
			 * \param config configuration of the UART (default : 9600 8 N 1)
			 */
			Uart(unsigned int id, ::uart::UartInterface::uart_callback callback, ::uart::Configuration config = ::uart::Configuration::_9600_8_N_1());

			/**
			 * \param send a C string
			 */
			virtual void send(const char* const str);

			/**
			 * \param send a buffer
			 */
			virtual void send(const types::buffer& buf);

			/**
			 * \param send a buffer
			 */
			virtual void send(const unsigned char* buf, unsigned int length);

		private:

			USART_TypeDef* _USARTx;
		};


	} /* namespace uart  */

} /* namespace stm32f1xx  */


namespace uart
{

	typedef ::stm32f1xx::uart::Uart ConcreteUart;

}
