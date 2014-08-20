#ifndef __SENSOR_H
#define __SENSOR_H
#include <stdint.h>
#include <avr/io.h>

#define VERSION_1_3 1

#define SENSOR_PORT PORTB
#define SENSOR_PIN  PINB
#define SENSOR_DDR  DDRB

#define SENSOR_GAP_CHANNEL 0x0C
#define SENSOR_INTERNAL_CHANNEL 0x0F

#ifdef VERSION_1_3
  #define SENSOR_SUPPLY 1
  #define PC_INT 1

  #define INT_VECT PCINT0_vect
  #define SENSOR_1_BIT 2
  #define SENSOR_1_CHANNEL 1

  #define SENSOR_2_BIT 3
  #define SENSOR_2_CHANNEL 3

  #define SENSOR_VCC_BIT 0

  #define RFM12_SEL_BIT 4
  #define RFM12_MISO_BIT 0
  #define RFM12_INT_BIT 1
  #define RFM12_MOSI_BIT 2
  #define RFM12_CLK_BIT 3

#else
  #ifdef VERSION_0_4
    #define INT_VECT INT0_vect
    #define SENSOR_1_BIT 4
    #define SENSOR_1_CHANNEL 2

    #define SENSOR_2_BIT 3
    #define SENSOR_2_CHANNEL 3

    #define SENSOR_VCC_BIT 9

    #define RFM12_SEL_BIT 0
    #define RFM12_MISO_BIT 1
    #define RFM12_INT_BIT 2
    #define RFM12_MOSI_BIT 4
    #define RFM12_CLK_BIT 3
  #else
    #define SENSOR_SUPPLY 1

    #define INT_VECT INT0_vect

    #define SENSOR_1_BIT 4
    #define SENSOR_2_BIT 3
    #define SENSOR_1_CHANNEL 2
    #define SENSOR_2_CHANNEL 3
    #define SENSOR_VCC_BIT 1

    #define RFM12_SEL_BIT 0
    #define RFM12_MISO_BIT 1
    #define RFM12_INT_BIT 2

    #ifdef V_2_0
      #define RFM12_MOSI_BIT 3
      #define RFM12_CLK_BIT 4
    #else
      #define RFM12_CLK_BIT 3
      #define RFM12_MOSI_BIT 4
    #endif
  #endif
#endif

#define SENSOR_VCC_OUT() SENSOR_DDR  |= (1 << SENSOR_VCC_BIT)
#define SENSOR_VCC_IN()  SENSOR_DDR  &=~(1 << SENSOR_VCC_BIT)
#define SENSOR_VCC_HI()  SENSOR_PORT |= (1 << SENSOR_VCC_BIT)
#define SENSOR_VCC_LOW() SENSOR_PORT &=~(1 << SENSOR_VCC_BIT)

#define SENSOR_1_OUT() SENSOR_DDR  |= (1 << SENSOR_1_BIT)
#define SENSOR_2_OUT() SENSOR_DDR  |= (1 << SENSOR_2_BIT)

#define SENSOR_1_IN() SENSOR_DDR &=~(1 << SENSOR_1_BIT)
#define SENSOR_2_IN() SENSOR_DDR &=~(1 << SENSOR_2_BIT)

#define SENSOR_1_LOW() SENSOR_PORT &=~(1 << SENSOR_1_BIT)
#define SENSOR_2_LOW() SENSOR_PORT &=~(1 << SENSOR_2_BIT)

#endif