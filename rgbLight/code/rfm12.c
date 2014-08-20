#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "rfm12.h"

uint8_t rfm12_nodeId = 101;
uint16_t rfm12_messageId = 0;
volatile uint8_t rfm12_state = 0;
volatile int8_t rfm12_bufferIndex = 0;
volatile rfm12_Message rfm12_tempBuffer;
volatile uint16_t rfm12_runningCRC;

#ifdef RFM12_USE_SPI
inline void rfm12_setSPISpeed(void) {
#if F_CPU <= 10000000
    SPCR = _BV(SPE) | _BV(MSTR);
#else
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0);
    SPSR |= _BV(SPI2X);
#endif
}

uint16_t rfm12Cmd (uint16_t cmd) {
  uint16_t reply;
  rfm12_setSPISpeed();

  RFM12_SEL_LOW();
  SPDR = cmd >> 8;
  while (!(SPSR & _BV(SPIF)));
  reply = SPDR << 8;
  SPDR = cmd;
  while (!(SPSR & _BV(SPIF)));
  reply |= SPDR;
  RFM12_SEL_HI();
  return reply;
}

#else
uint16_t rfm12Cmd (uint16_t cmd) {
  uint8_t i;
  uint16_t reply = 0;

  RFM12_CLK_LOW();
  RFM12_SEL_LOW();
  for(i = 0; i < 16; i++) {
    RFM12_MOSI_LOW();
    if(cmd & 0x8000) {
      RFM12_MOSI_HI();
    }
    _delay_us(0.1);
    RFM12_CLK_HI();
    reply <<= 1;
    reply |= RFM12_MISO_READ();
    _delay_us(0.1);
    RFM12_CLK_LOW();
    cmd <<= 1;
  }
  RFM12_SEL_HI();
  return reply;
}
#endif

inline uint8_t rfm12_getState(void){
  return rfm12_state;
}

inline volatile rfm12_Message* rfm12_getMessage(void){
  return &rfm12_tempBuffer;
}


void rfm12_setupPins(void){
  RFM12_MISO_HI();
  RFM12_SEL_HI();
  RFM12_CLK_LOW();
  RFM12_MOSI_HI();
  RFM12_INT_HI();

  RFM12_SEL_OUT();
  RFM12_CLK_OUT();
  RFM12_MOSI_OUT();
  RFM12_INT_IN();
  RFM12_MISO_IN();

}

uint8_t rfm12_loadNodeId(void) {
  uint8_t temp = eeprom_read_byte((uint8_t*) RFM12_SAVE_CHECK_ADDR);
  if(temp == RFM12_SAVE_CHECK_VALUE) {
    return eeprom_read_byte((uint8_t*) RFM12_SAVE_ID_ADDR);
  }
  return 0;
}

void rfm12_saveNodeId(uint8_t nodeId) {
  rfm12_nodeId = nodeId;
  eeprom_write_byte((uint8_t*) RFM12_SAVE_CHECK_ADDR, RFM12_SAVE_CHECK_VALUE);
  eeprom_write_byte((uint8_t*) RFM12_SAVE_ID_ADDR, nodeId);
}


void rfm12_init(uint8_t band) {
  rfm12_nodeId = rfm12_loadNodeId();
  if(rfm12_nodeId == 0){
    rfm12_nodeId = RFM12_DEFAULT_ID;
  }
  rfm12_setupPins();

  // Init RFM12 module
  rfm12Cmd(0x0000); // intitial SPI transfer added to avoid power-up problem
  rfm12Cmd(RFM12_SLEEP_MODE); // DC (disable clk pin), enable lbd

  while (RFM12_INT_READ() == 0)
    rfm12Cmd(0x0000);

  rfm12Cmd(0x80C7 | (band << 4)); // FIFOs acivated, 12.0pF
  rfm12Cmd(0xA640); // 868MHz
  rfm12Cmd(0xC606); // approx 49.2 Kbps, i.e. 10000/29/(1+6) Kbps
  rfm12Cmd(0x94A2); // VDI,FAST,134kHz,0dBm,-91dBm
  rfm12Cmd(0xC2AC); // AL,!ml,DIG,DQD4
  rfm12Cmd(0xCA83); // FIFO8,SYNC,!ff,DR
//  rfm12Cmd(0xCAF3); // FIFO8,SYNC,!ff,DR
  rfm12Cmd(0xCE00 | 212); // SYNC=2DXX?
  rfm12Cmd(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN
  rfm12Cmd(0x9850); // !mp,90kHz,MAX OUT
  rfm12Cmd(0xCC77); // OB1?OB0, LPX,?ddy?DDIT?BW0
  rfm12Cmd(0xE000); // NOT USE
  rfm12Cmd(0xC800); // NOT USE
  rfm12Cmd(0xC0E9); // 10MHz,3.1V

  rfm12_messageId = 0;
  rfm12_setIdle();
}

void rfm12_setIdle(void) {
  rfm12Cmd(RFM12_IDLE_MODE);
  rfm12Cmd(0x0000);
  rfm12_state = RFM12_IDLE;
}

void rfm12_senderOn(void) {
  rfm12_state = RFM12_SENDING;
  rfm12Cmd(RFM12_XMITTER_ON);
  rfm12Cmd(0x0000);
}

inline void rfm12_disableInterrupt(void) {
  GIMSK &= ~( 1 << RFM12_INT_REG);
}

inline void rfm12_enableInterrupt(void) {
  GIMSK |= ( 1 << RFM12_INT_REG);
}

/*
uint8_t* rfm12_recieveBlocking(void) {
  uint8_t fifoData;
  uint8_t index = 0;

  uart_puts(".");
  while (RFM12_INT_READ() == 0) {
    rfm12Cmd(0x0000);
    rfm12Cmd(RFM12_TXREG_WRITE + 0xAA);
    rfm12_setIdle();
  }
  uart_puts("!");

  rfm12_state = RFM12_RECIEVING;
  rfm12Cmd(RFM12_RECEIVER_ON);

  while(index < RFM12_BUF_LEN) {
    while (RFM12_INT_READ()) {
      rfm12Cmd(0x0000);
      // wait until we recieve something
    }
    fifoData = rfm12Cmd(RFM12_RX_FIFO_READ);
    rfm12_buffer[index] = fifoData;
    index++;
    uart_puts("o");
  }
  rfm12_setIdle();
  return rfm12_buffer;
}
*/

void rfm12_recieverOn(void) {
  rfm12_tempBuffer.errors = RFM_12_ERROR_INIT;
  rfm12_tempBuffer.flags = 0;
  rfm12_tempBuffer.sender = 0;
  rfm12_tempBuffer.recipient = 0;
  rfm12_tempBuffer.type = RFM12_MESSAGE_TYPE_INVALID;
  rfm12_tempBuffer.crc = 0xAAAA;
  rfm12_runningCRC = 0xAAAA;

  rfm12_bufferIndex = -RFM12_HEADER_LENGTH;

  rfm12_state = RFM12_RECIEVING;
  rfm12Cmd(RFM12_RECEIVER_ON);
  rfm12Cmd(0x0000);
}


inline void rfm12_sendByte(uint8_t data){
  while(RFM12_INT_READ());     //wait until data in buffer
  rfm12Cmd(RFM12_TXREG_WRITE + data);
}

uint16_t rfm12_sendByteCRC(uint8_t data, uint16_t crc){
  rfm12_sendByte(data);
  return _crc16_update(crc, data);
}

void rfm12_sendBlocking(uint8_t recipient, uint8_t* data, uint8_t length, uint8_t type, uint8_t flags){
  uint16_t crc = 0xAAAA;
  uint8_t i;
  rfm12_messageId++;

  rfm12_senderOn();
  // Preamble
  rfm12_sendByte(0xAA);
  rfm12_sendByte(0xAA);
  rfm12_sendByte(0xAA);
  rfm12_sendByte(0x2D);
  rfm12_sendByte(0xD4);

  // Length
  crc = rfm12_sendByteCRC(length + RFM12_HEADER_LENGTH + RFM12_CRC_LENGTH, crc);
  // ID
  crc = rfm12_sendByteCRC(rfm12_nodeId, crc);
  // Recipient
  crc = rfm12_sendByteCRC(recipient, crc);
  // Flags
  crc = rfm12_sendByteCRC(flags, crc);
  // Type
  crc = rfm12_sendByteCRC(type, crc);
  // Message Id
  crc = rfm12_sendByteCRC(rfm12_messageId >> 8, crc);
  crc = rfm12_sendByteCRC(rfm12_messageId & 0xFF, crc);

  for(i = 0 ; i < length; i++) {
    crc = rfm12_sendByteCRC(data[i], crc);
  }

  //CRC
  rfm12_sendByte(crc >> 8);
  rfm12_sendByte(crc & 0xFF);

  // Just some buffering
  rfm12_sendByte(0xAA);
  rfm12_sendByte(0xAA);
  rfm12_sendByte(0xAA);
  rfm12_sendByte(0xAA);

  rfm12_setIdle();
}

void rfm12_replyAck(void){
  uint8_t data[2];
  data[0] = rfm12_tempBuffer.messageId >> 8;
  data[1] = rfm12_tempBuffer.messageId & 0xFF;
  rfm12_sendBlocking(rfm12_tempBuffer.sender, data, 2, RFM12_MESSAGE_TYPE_SYS_PONG, 0);
}

static inline void rfm12_interruptRec(void) {
  uint8_t fifoData = rfm12Cmd(RFM12_RX_FIFO_READ);
  switch(rfm12_bufferIndex) {
    // Length
    case -7:
      rfm12_tempBuffer.errors = 0;
      rfm12_tempBuffer.length = fifoData - (RFM12_HEADER_LENGTH + RFM12_CRC_LENGTH);
      if(fifoData < (RFM12_HEADER_LENGTH + RFM12_CRC_LENGTH)) {
        rfm12_tempBuffer.errors |= RFM_12_ERROR_LENGTH;
        rfm12_tempBuffer.errors |= RFM_12_ERROR_SHORT;
        rfm12_setIdle();
      } else {
        if (fifoData > RFM12_PACKET_LENGTH){
          rfm12_tempBuffer.errors |= RFM_12_ERROR_LENGTH;
          rfm12_tempBuffer.errors |= RFM_12_ERROR_LONG;
          rfm12_setIdle();
        }
      }
      break;

    // Sender
    case -6:
      rfm12_tempBuffer.sender = fifoData;
      break;

    // Recipient
    case -5:
      rfm12_tempBuffer.recipient = fifoData;
      if((rfm12_tempBuffer.recipient != rfm12_nodeId) && (rfm12_tempBuffer.recipient != RFM12_BROADCAST_ID)) {
        rfm12_tempBuffer.errors |= RFM_12_ERROR_RECIP;
      }
      break;

    // Flags
    case -4:
      rfm12_tempBuffer.flags = fifoData;
      break;

    // Type
    case -3:
      rfm12_tempBuffer.type = fifoData;
      break;

    // Message Id
    case -2:
      rfm12_tempBuffer.messageId = fifoData << 8;
      break;

    // Message Id
    case -1:
      rfm12_tempBuffer.messageId |= fifoData;
      break;

    default:
      if(rfm12_bufferIndex < rfm12_tempBuffer.length){
        rfm12_tempBuffer.data[rfm12_bufferIndex] = fifoData;
      } else {
        switch(rfm12_bufferIndex - rfm12_tempBuffer.length) {
          case 0:
            rfm12_runningCRC = fifoData << 8;
            rfm12_bufferIndex++;
            break;

          case 1:
            rfm12_setIdle();
            rfm12_runningCRC |= fifoData;
            if((rfm12_runningCRC != rfm12_tempBuffer.crc) && !(rfm12_tempBuffer.flags & RFM12_FLAG_IGNORE_CRC)) {
              rfm12_tempBuffer.errors |= RFM_12_ERROR_CRC;
            }
            rfm12_bufferIndex++;
            break;

          default:
            rfm12_setIdle();
            break;
        }
        // return early, so we don't update the crc
        return;
      }
      break;
  }
  rfm12_bufferIndex++;
  rfm12_tempBuffer.crc = _crc16_update(rfm12_tempBuffer.crc, fifoData);
}


ISR(INT0_vect) {
//  rfm12Cmd(0x0000);
  switch (rfm12_state) {
    case RFM12_RECIEVING:
      rfm12_interruptRec();
      break;

    default:
      // I have no idea why we are here, so clear the fifo, fill the send queue and set it to idle
      rfm12Cmd(RFM12_RX_FIFO_READ);
      rfm12Cmd(RFM12_TXREG_WRITE+0xAA);
      rfm12_setIdle();
      break;
  }
}
