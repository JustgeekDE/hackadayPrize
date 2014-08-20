#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "light.h"
#include "pins.h"

static volatile uint32_t pwm_time = 0;


//PWM Table from mikrocontroller.net
uint8_t light_pwmtable[LIGHT_PWM_TABLE_SIZE] PROGMEM = {0,   1,   2,   2,   2,   3,   3,   4,   5 ,  6,
                                                   7,   8,  10,  11,  13,  16,  19,  23,  27,  32,
                                                  38,  45,  54,  64,  76,  91, 108, 128, 152, 181, 215, 255};


ISR(TIM0_COMPA_vect){
  OCR0A += TIME_INTERVALL;
  pwm_time += 1;
}

uint32_t pwm_getTime(void){
  return pwm_time;
}

uint8_t light_convLin(uint8_t value) {
  uint8_t value1, value2, remainder;
  uint8_t tableIndex;

  remainder = value & LIGHT_PWM_TABLE_REMAINDER;

  tableIndex = (value >> LIGHT_PWM_TABLE_BITS);
  value1 = pgm_read_word(light_pwmtable + tableIndex);

  if(remainder && (value1 < LIGHT_PWM_MAX) && ((tableIndex+1) < LIGHT_PWM_TABLE_SIZE)) {
    value2 = pgm_read_word(light_pwmtable + tableIndex + 1);
    if(value1 != value2) {

      // linear interpolation
      int16_t temp;

      temp = value2 - value1;
      temp *= remainder;
      temp = temp >> LIGHT_PWM_REMAINING_BITS;
      temp += value1;
      temp = temp > LIGHT_PWM_MAX ? LIGHT_PWM_MAX : temp < 0 ? 0 : temp;

      value1 = temp;
    }
  }
  return value1;
}



void light_setValue(uint8_t r, uint8_t g, uint8_t b, uint8_t lin) {
	if(lin) {
		r = light_convLin(r);
		g = light_convLin(g);
		b = light_convLin(b);
	}

  if(r == 0) {
    CLEAR_BIT(RGB1_CTRL_REG, RGB1_CTRL_BIT);
    CLEAR_BIT(RGB1_PORT, RGB1_BIT);
  } else {
    SET_BIT(RGB1_CTRL_REG, RGB1_CTRL_BIT);
  }

  if(g == 0) {
    CLEAR_BIT(RGB2_CTRL_REG, RGB2_CTRL_BIT);
    CLEAR_BIT(RGB2_PORT, RGB2_BIT);
  } else {
    SET_BIT(RGB2_CTRL_REG, RGB2_CTRL_BIT);
  }

  if(b == 0) {
    CLEAR_BIT(RGB3_CTRL_REG, RGB3_CTRL_BIT);
    CLEAR_BIT(RGB3_PORT, RGB3_BIT);
  } else {
    SET_BIT(RGB3_CTRL_REG, RGB3_CTRL_BIT);
  }

  RGB1_TIMER_REG = r;
  RGB2_TIMER_REG = g;
  RGB3_TIMER_REG = b;

}


uint8_t light_setup(void) {
  // Timer 0 PWM Init
  // Fast PWM max 0xFF, clear on match, full clock
  TCCR0A = (1 << WGM00)
         | (1 << WGM01)
         | (1 << COM0B1);

  TCCR0B = (0 << WGM02)
         | (1 << CS00)
         | (1 << CS01)
         | (0 << CS02);

  // Timer 1 PWM Init
  // Fast PWM max 0xFF, clear on match, full clock
  TCCR1A = (1 << WGM10)
         | (0 << WGM12)
         | (1 << COM1A1)
         | (1 << COM1B1);

  TCCR1B = (1 << WGM12)
         | (1 << CS10)
         | (0 << CS11)
         | (0 << CS12);

  SET_BIT(RGB1_DDR, RGB1_BIT);
  SET_BIT(RGB2_DDR, RGB2_BIT);
  SET_BIT(RGB3_DDR, RGB3_BIT);

  //Activate Interrupt 1 for timer ever ms;
  OCR0A = TIME_INTERVALL;
  TIMSK0 |= (1<<OCIE0A);

  light_setValue(0,0,0,0);
  return 1;
}

