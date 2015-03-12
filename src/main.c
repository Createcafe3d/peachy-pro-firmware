#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"

#include "iolib.h"
#include "serialio.h"
#include "pwmout.h"
#include "dripper.h"

#include <usb_core.h>
#include <usb_cdc.h>
#include <i2c.h>

extern volatile uint32_t g_dripcount;

static volatile uint32_t tick = 0;

void delay_ms(int ms) {
  uint32_t end = tick + (ms*2);
  while(tick < end);
}

void SysTick_Handler(void) {
  tick += 1;
  static int tick_count = 0;
  if (tick_count > 2000) {
//    if(!WouldTxBlock()) { QueueTx(msg, 7); }
    tick_count = 0;
  }
  tick_count++;

  update_pwm();
}

int main(void)
{
  USB_Start();
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

  // Set Up PA4 and PA5 as debug pins
  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gp);

  gp.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &gp);

  i2c_init();
  initialize_pwm();
  initialize_dripper();

  GPIO_WriteBit(GPIOB, GPIO_Pin_12, 1);
  GPIO_WriteBit(GPIOB, GPIO_Pin_13, 1);
  GPIO_WriteBit(GPIOB, GPIO_Pin_14, 1);
  GPIO_WriteBit(GPIOB, GPIO_Pin_15, 1);
  
  SysTick_Config(SystemCoreClock / 2000);

  int last_drip_count = g_dripcount;
  while(1) {
    serialio_feed();

    if (g_dripcount != last_drip_count) {
      last_drip_count = g_dripcount;
      send_updated_drip_count();
    }
  }
}
