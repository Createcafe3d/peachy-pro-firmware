#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"

#include "iolib.h"
#include "serialio.h"
#include "pwmout.h"
#include "dripper.h"
#include "reprog.h"
#include "hwaccess.h"
#include "clock.h"

#include <usb_core.h>
#include <usb_cdc.h>
#include <i2c.h>

extern volatile uint32_t g_dripcount;
extern volatile uint32_t g_dripghosts;
extern volatile uint32_t g_adcVals[ADC_CHANS];
extern volatile uint16_t g_adcCal;

volatile uint32_t tick = 0;
bool g_debug=0;
bool g_checkcoils=1;
bool g_twig_coils=1;
uint8_t move_start = 0;
uint8_t move_count = 0;
Move move_buffer[MOVE_SIZE];

void delay_ms(int ms) {
  uint32_t end = tick + (ms*2);
  while(tick < end);
}

void SysTick_Handler(void) {
  tick += 1;
  update_pwm();
  updateADC();
  if(g_twig_coils){
    twigCoils();
  }
	if (g_debug){
		laserToggleTest();
	}
}

void init_serial_number() {
  uint32_t* serial_base = (uint32_t*)0x1FFFF7AC;
  // 96-bit serial number = 4 words
  uint32_t part1 = *serial_base;
  uint32_t part2 = *(serial_base+1);
  uint32_t part3 = *(serial_base+2);
  uint32_t part4 = *(serial_base+3);
  
  uint32_t hashed = part1 ^ part2 ^ part3 ^ part4;
  USB_SetSerial(hashed);
  set_identify_serial_number(hashed);
}

int main(void)
{
  init_serial_number();
	USB_Start();
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	setupJP5();
	setupJP6();
	setupLeds();

  //i2c_init();
  initialize_pwm();
  initialize_dripper();
  initialize_debouncer();

  //GPIO_WriteBit(GPIOB, GPIO_Pin_12, 1); //Controlled by USB software
	setCornerLed(1);
	setInLed(0);
	setCoilLed(0);
  
  SysTick_Config(SystemCoreClock / 2000); //48MHz/2000 gives us 24KHz, so a count of 24000 should be 1 second

  //TODO:
  //Twig coils until first move message
  //Use the systick variable and set when next instruction is
  //use switch case for state machine?
  //move check inside the wile loop
  //if (g_checkcoils){
    //checkCoils();
  //}

  int last_drip_count = g_dripcount;
  while(1) {
    serialio_feed();

    if (move_count!=0){
      g_twig_coils=0;
    }

    if (g_dripcount != last_drip_count) {
      last_drip_count = g_dripcount;
      send_updated_drip_count();
    }
    if (g_adcVals[2] != 0){
      setInLed(1);
    }
    else{
      setInLed(0);
    }
  }
}
