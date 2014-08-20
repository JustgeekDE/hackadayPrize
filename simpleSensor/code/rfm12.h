#ifndef __RFM12_H
#define __RFM12_H
#include <stdint.h>
#include <avr/io.h>
#include "sensor.h"


#define RFM12_SAVE_VERSION 1
#define RFM12_SAVE_OFFSET 0

// RF12 command codes
#define RF_RECEIVER_ON  0x82D9
#define RF_XMITTER_ON   0x8239
//#define RF_IDLE_MODE    0x8209
#define RF_IDLE_MODE    0x8201
#define RF_SLEEP_MODE   0x8201
#define RF_WAKEUP_MODE  0x8203
#define RF_TXREG_WRITE  0xB800
#define RF_RX_FIFO_READ 0xB000
#define RF_WAKEUP_TIMER 0xE000

#define RFM12_433MHZ 1
#define RFM12_868MHZ 2
#define RFM12_915MHZ 3

#define RFM12_MESSAGE_TYPE_PLAIN       0x01
#define RFM12_MESSAGE_TYPE_SET_NODE_ID 0x02
#define RFM12_MESSAGE_TYPE_SENSOR_DATA 0x10
#define RFM12_MESSAGE_TYPE_RAW_DATA    0x11
#define RFM12_MESSAGE_TYPE_TEMP_DATA   0x12
#define RFM12_MESSAGE_TYPE_ELRO_CMD    0x20
#define RFM12_MESSAGE_TYPE_NET_CMD     0x30

#define RFM12_FLAG_IGNORE_CRC 0x01

#define RFM_12_ERROR_LENGTH 0x01
#define RFM_12_ERROR_CRC    0x02
#define RFM_12_ERROR_RECIP  0x08

#define RFM12_BROADCAST_ID 0
#define RFM12_ID 5


#define RFM12_INT_PORT PORTB
#define RFM12_INT_PIN  PINB
#define RFM12_INT_DDR  DDRB

#define RFM12_CLK_PORT PORTB
#define RFM12_CLK_PIN  PINB
#define RFM12_CLK_DDR  DDRB

#define RFM12_SEL_PORT PORTB
#define RFM12_SEL_PIN  PINB
#define RFM12_SEL_DDR  DDRB

#define RFM12_MOSI_PORT PORTB
#define RFM12_MOSI_PIN  PINB
#define RFM12_MOSI_DDR  DDRB

#define RFM12_MISO_PORT PORTB
#define RFM12_MISO_PIN  PINB
#define RFM12_MISO_DDR  DDRB


#define RFM12_CLK_LOW()  RFM12_CLK_PORT &=~(1 << RFM12_CLK_BIT)
#define RFM12_CLK_HI()   RFM12_CLK_PORT |= (1 << RFM12_CLK_BIT)
#define RFM12_CLK_OUT()  RFM12_CLK_DDR  |= (1 << RFM12_CLK_BIT)
#define RFM12_CLK_IN()   RFM12_CLK_DDR  &=~(1 << RFM12_CLK_BIT)

#define RFM12_MOSI_LOW() RFM12_MOSI_PORT &=~(1 << RFM12_MOSI_BIT)
#define RFM12_MOSI_HI()  RFM12_MOSI_PORT |= (1 << RFM12_MOSI_BIT)
#define RFM12_MOSI_OUT() RFM12_MOSI_DDR  |= (1 << RFM12_MOSI_BIT)
#define RFM12_MOSI_IN()  RFM12_MOSI_DDR  &=~(1 << RFM12_MOSI_BIT)

#define RFM12_SEL_LOW() RFM12_SEL_PORT &=~(1 << RFM12_SEL_BIT)
#define RFM12_SEL_HI()  RFM12_SEL_PORT |= (1 << RFM12_SEL_BIT)
#define RFM12_SEL_OUT() RFM12_SEL_DDR  |= (1 << RFM12_SEL_BIT)
#define RFM12_SEL_IN()  RFM12_SEL_DDR  &=~(1 << RFM12_SEL_BIT)

#define RFM12_MISO_LOW()  RFM12_MISO_PORT &=~(1 << RFM12_MISO_BIT)
#define RFM12_MISO_HI()   RFM12_MISO_PORT |= (1 << RFM12_MISO_BIT)
#define RFM12_MISO_IN()   RFM12_MISO_DDR  &=~(1 << RFM12_MISO_BIT)
#define RFM12_MISO_READ() (RFM12_MISO_PIN&(1 << RFM12_MISO_BIT) ? 1 : 0)

#define RFM12_INT_LOW()  RFM12_INT_PORT &=~(1 << RFM12_INT_BIT)
#define RFM12_INT_HI()   RFM12_INT_PORT |= (1 << RFM12_INT_BIT)
#define RFM12_INT_IN()   RFM12_INT_DDR &=~(1 << RFM12_INT_BIT)
#define RFM12_INT_READ()   (RFM12_INT_PIN&(1 << RFM12_INT_BIT) ? 1 : 0)

#define RFM12_IDLE 0
#define RFM12_RECIEVING 1
#define RFM12_SENDING 2
#define RFM12_SLEEPING 3

#define RFM12_PREAMBLE_LENGTH 5
#define RFM12_HEADER_LENGTH 7
#define RFM12_CRC_LENGTH 2

extern void rfm12Cmd (uint16_t cmd);
extern void rfm12_sendByte(uint8_t data);
extern void rfm12_sendBuffer(uint8_t recipient, uint8_t* data, uint8_t length, uint8_t type, uint8_t flags);
extern void rfm12_init(uint8_t nodeId, uint8_t band);
extern void rfm12_setupPins(void);
extern void rfm12_setIdle(void);
extern void rfm12_senderOn(void);

extern void rfm12_interrupt(void);
extern void rfm12_startSleep(uint8_t n);
#endif