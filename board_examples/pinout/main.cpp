/******************************************************************************\
		INCLUDES
\******************************************************************************/
#include <hal/Clock.hpp>
#include <Pinout.hpp>

/******************************************************************************\
		DEFINES
\******************************************************************************/
#define LED_PORT							6
#define LED_PIN								1

/******************************************************************************\
		GLOBALS
\******************************************************************************/
::pinout::ConcretePinout pinLed(LED_PORT, LED_PIN);

/******************************************************************************\
		FUNCTIONS
\******************************************************************************/
int main(void)
{

  int			time;

  time = 500;
  while(1)
  {
	//restart counter
	if (time < 10) time = 500;

	//set led high
	pinLed.high();

	//wait
    clock::msleep(time);

    //set led low
    pinLed.low();

    //wait
    clock::msleep(time);

    //decrease delay
    time -= 20;

  }

  while (1);
}

//------------------------------------------------------------------------------

