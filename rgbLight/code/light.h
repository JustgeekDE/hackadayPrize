#ifndef __LIGHT_H
#define __LIGHT_H
#include <stdint.h>

#include <avr/io.h>

#define TIME_INTERVALL 78


#define LIGHT_PWM_TOTAL_BITS 8
#define LIGHT_PWM_TABLE_BITS 5
#define LIGHT_PWM_REMAINING_BITS (LIGHT_PWM_TOTAL_BITS - LIGHT_PWM_TABLE_BITS)

#define LIGHT_PWM_MAX ((1 << LIGHT_PWM_TOTAL_BITS) - 1)
#define LIGHT_PWM_TABLE_SIZE (1<<LIGHT_PWM_TABLE_BITS)
#define LIGHT_PWM_TABLE_REMAINDER ((1 << LIGHT_PWM_REMAINING_BITS) -1)

#define RGB1_PORT PORTA
#define RGB1_DDR  DDRA
#define RGB1_BIT  PA6
#define RGB1_TIMER_REG OCR1A
#define RGB1_CTRL_REG TCCR1A
#define RGB1_CTRL_BIT COM1A1

#define RGB2_PORT PORTA
#define RGB2_DDR  DDRA
#define RGB2_BIT  PA5
#define RGB2_TIMER_REG OCR1B
#define RGB2_CTRL_REG TCCR1A
#define RGB2_CTRL_BIT COM1B1

#define RGB3_PORT PORTA
#define RGB3_DDR  DDRA
#define RGB3_BIT  PA7
#define RGB3_TIMER_REG OCR0B
#define RGB3_CTRL_REG TCCR0A
#define RGB3_CTRL_BIT COM0B1

extern uint8_t light_setup(void);

extern void light_setValue(uint8_t r, uint8_t g, uint8_t b, uint8_t linearMode);

extern uint32_t pwm_getTime(void);

#endif