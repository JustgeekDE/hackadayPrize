#ifndef __RFM12_H
#define __RFM12_H
#include <stdint.h>
#include <avr/io.h>

// Addresses and checksums to save node id in eeprom
#define RFM12_SAVE_CHECK_ADDR  0x01
#define RFM12_SAVE_CHECK_VALUE 0xAA
#define RFM12_SAVE_ID_ADDR     0x02

// Default id, if no saved id is found
#define RFM12_DEFAULT_ID 250
#define RFM12_BROADCAST_ID 0

#ifdef old_power
  // RF12 command codes
  #define RFM12_RECEIVER_ON  0x82D9
  #define RFM12_XMITTER_ON   0x8239
  #define RFM12_IDLE_MODE    0x8201
  #define RFM12_SLEEP_MODE   0x8201
  #define RFM12_WAKEUP_MODE  0x8203
  #define RFM12_TXREG_WRITE  0xB800
  #define RFM12_RX_FIFO_READ 0xB000
  #define RFM12_WAKEUP_TIMER 0xE000
#else
  // RF12 command codes
  #define RFM12_RECEIVER_ON  0x82D8
  #define RFM12_XMITTER_ON   0x8238
  #define RFM12_IDLE_MODE    0x8208
  #define RFM12_SLEEP_MODE   0x8208
  #define RFM12_WAKEUP_MODE  0x820A
  #define RFM12_TXREG_WRITE  0xB800
  #define RFM12_RX_FIFO_READ 0xB000
  #define RFM12_WAKEUP_TIMER 0xE000
#endif

// Frequency commands
#define RFM12_433MHZ 1
#define RFM12_868MHZ 2
#define RFM12_915MHZ 3

// Message types
#define RFM12_MESSAGE_TYPE_INVALID     0x00
#define RFM12_MESSAGE_TYPE_PLAIN       0x01
#define RFM12_MESSAGE_TYPE_SET_NODE_ID 0x02
#define RFM12_MESSAGE_TYPE_SENSOR_DATA 0x10
#define RFM12_MESSAGE_TYPE_RAW_DATA    0x11
#define RFM12_MESSAGE_TYPE_TEMP_DATA   0x12
#define RFM12_MESSAGE_TYPE_ELRO_CMD    0x20
#define RFM12_MESSAGE_TYPE_NET_CMD     0x30
#define RFM12_MESSAGE_TYPE_RGB_DATA    0x40
#define RFM12_MESSAGE_TYPE_RGB_LIN     0x41
#define RFM12_MESSAGE_TYPE_RGB_ANIM    0x42
#define RFM12_MESSAGE_TYPE_SYS_PONG    0x81

// Message Flags
#define RFM12_FLAG_IGNORE_CRC 0x01
#define RFM12_FLAG_REQ_ACK    0x02

// Error flags
#define RFM_12_ERROR_LENGTH 0x01
#define RFM_12_ERROR_CRC    0x02
#define RFM_12_ERROR_INIT   0x04
#define RFM_12_ERROR_RECIP  0x08
#define RFM_12_ERROR_SHORT  0x10
#define RFM_12_ERROR_LONG   0x20

// Hardware settings
#define RFM12_INT_BIT 2
#define RFM12_CLK_BIT 4
#define RFM12_SEL_BIT 3
#define RFM12_MOSI_BIT 2
#define RFM12_MISO_BIT 1

#define RFM12_INT_PORT PORTB
#define RFM12_INT_PIN  PINB
#define RFM12_INT_DDR  DDRB

#define RFM12_CLK_PORT PORTA
#define RFM12_CLK_PIN  PINA
#define RFM12_CLK_DDR  DDRA

#define RFM12_SEL_PORT PORTA
#define RFM12_SEL_PIN  PINA
#define RFM12_SEL_DDR  DDRA

#define RFM12_MOSI_PORT PORTA
#define RFM12_MOSI_PIN  PINA
#define RFM12_MOSI_DDR  DDRA

#define RFM12_MISO_PORT PORTB
#define RFM12_MISO_PIN  PINB
#define RFM12_MISO_DDR  DDRB

#define RFM12_INT_REG INT0

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

// Status flags
#define RFM12_IDLE 0
#define RFM12_RECIEVING 1
#define RFM12_SENDING 2
#define RFM12_SLEEPING 3

#define RFM12_PREAMBLE_LENGTH 5
#define RFM12_HEADER_LENGTH 7
#define RFM12_CRC_LENGTH 2

#define RFM12_BUFFER_LENGTH 16
#define RFM12_PACKET_LENGTH (RFM12_BUFFER_LENGTH + RFM12_HEADER_LENGTH + RFM12_CRC_LENGTH)

typedef struct {
  uint16_t crc;
  uint16_t messageId;
  uint8_t data[RFM12_BUFFER_LENGTH+1];
  uint8_t sender;
  uint8_t recipient;
  uint8_t length;
  uint8_t flags;
  uint8_t type;
  uint8_t errors;
} rfm12_Message;


extern volatile rfm12_Message* rfm12_getMessage(void);
extern uint8_t rfm12_getState(void);

extern void rfm12_init(uint8_t band);
extern void rfm12_setIdle(void);

extern void rfm12_recieverOn(void);
extern void rfm12_sendBlocking(uint8_t recipient, uint8_t* data, uint8_t length, uint8_t type, uint8_t flags);

extern void rfm12_replyAck(void);

extern uint8_t rfm12_loadNodeId(void);
extern void rfm12_saveNodeId(uint8_t nodeId);

extern void rfm12_disableInterrupt(void);
extern void rfm12_enableInterrupt(void);

#endif