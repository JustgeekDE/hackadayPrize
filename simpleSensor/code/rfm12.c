#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

#include "rfm12.h"

static uint8_t rfm12_nodeId = 0;
static uint16_t rfm12_messageId = 0;
static uint8_t rfm12_state = 0;


void rfm12Cmd (uint16_t cmd) {
  uint8_t i;

  RFM12_SEL_LOW();
  for(i = 0; i < 16; i++) {
    if(cmd & 0x8000) {
      RFM12_MOSI_HI();
    }
    else {
      RFM12_MOSI_LOW();
    }
    RFM12_CLK_HI();
    cmd <<= 1;
    RFM12_CLK_LOW();
  }
  RFM12_SEL_HI();
}

void rfm12_sendByte(uint8_t data){
  while(RFM12_INT_READ());     //wait until data in buffer
  rfm12Cmd(RF_TXREG_WRITE + data);
}


void rfm12_sendBuffer(uint8_t recipient, uint8_t* data, uint8_t length, uint8_t type, uint8_t flags){
  uint16_t crc = 0xAAAA;
  uint8_t i,value = 0;
  rfm12_messageId++;
  //Precalculate CRC to make this fast enough for 1 mHz
  crc = _crc16_update(crc, length + RFM12_HEADER_LENGTH + RFM12_CRC_LENGTH);
  crc = _crc16_update(crc, rfm12_nodeId);
  crc = _crc16_update(crc, recipient);
  crc = _crc16_update(crc, flags);
  crc = _crc16_update(crc, type);
  crc = _crc16_update(crc, rfm12_messageId >> 8);
  crc = _crc16_update(crc, rfm12_messageId & 0xFF);
  for(i = 0 ; i < length; i++) {
    crc = _crc16_update(crc, data[i]);
  }


  rfm12_senderOn();
  // Preamble
  rfm12_sendByte(0xAA);
  rfm12_sendByte(0xAA);
  rfm12_sendByte(0xAA);
  rfm12_sendByte(0x2D);
  rfm12_sendByte(0xD4);

  // Length
  value = length + RFM12_HEADER_LENGTH + RFM12_CRC_LENGTH;
  rfm12_sendByte(value);
  // ID
  rfm12_sendByte(rfm12_nodeId);
  // Recipient
  rfm12_sendByte(recipient);
  // Flags
  rfm12_sendByte(flags);
  // Type
  rfm12_sendByte(type);
  // Message Id
  rfm12_sendByte(rfm12_messageId >> 8);
  rfm12_sendByte(rfm12_messageId & 0xFF);

  for(i = 0 ; i < length; i++) {
    rfm12_sendByte(data[i]);
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


void rfm12_setupPins(void){
  RFM12_SEL_OUT();
  RFM12_CLK_OUT();
  RFM12_MOSI_OUT();
  RFM12_INT_IN();
//  RFM12_MISO_IN();

  RFM12_SEL_HI();
  RFM12_CLK_LOW();
  RFM12_MOSI_HI();
  RFM12_INT_HI();
}

void rfm12_clearPins(void){
  RFM12_SEL_IN();
  RFM12_CLK_IN();
  RFM12_MOSI_IN();

  RFM12_SEL_HI();
  RFM12_CLK_HI();
  RFM12_MOSI_HI();
  RFM12_INT_HI();
//  RFM12_MISO_HI();
}

void rfm12_init(uint8_t nodeId, uint8_t band) {
  rfm12_nodeId = nodeId;

  rfm12_setupPins();

  // Init RFM12 module
  rfm12Cmd(0x0000); // intitial SPI transfer added to avoid power-up problem
  rfm12Cmd(RF_SLEEP_MODE); // DC (disable clk pin), enable lbd

  while (RFM12_INT_READ() == 0)
    rfm12Cmd(0x0000);

  rfm12Cmd(0x80C7 | (band << 4)); // FIFOs acivated, 12.0pF
  rfm12Cmd(0xA640); // 868MHz
  rfm12Cmd(0xC606); // approx 49.2 Kbps, i.e. 10000/29/(1+6) Kbps
  rfm12Cmd(0x94A2); // VDI,FAST,134kHz,0dBm,-91dBm
  rfm12Cmd(0xC2AC); // AL,!ml,DIG,DQD4
  rfm12Cmd(0xCAF3); // FIFO8,SYNC,!ff,DR
  rfm12Cmd(0xCE00 | 212); // SYNC=2DXX?
  rfm12Cmd(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN
  rfm12Cmd(0x9850); // !mp,90kHz,MAX OUT
  rfm12Cmd(0xCC77); // OB1?OB0, LPX,?ddy?DDIT?BW0
//  rfm12Cmd(0xCC10); // PLL low power mode
  rfm12Cmd(0xE000); // NOT USE
  rfm12Cmd(0xC800); // NOT USE
  rfm12Cmd(0xC049); // 1.66MHz,3.1V

  rfm12_messageId = 0;
  rfm12_setIdle();
}

void rfm12_setIdle(void) {
  rfm12Cmd(RF_IDLE_MODE);
  rfm12_state = RFM12_IDLE;
}


void rfm12_senderOn(void) {
  rfm12_state = RFM12_SENDING;
  rfm12Cmd(RF_XMITTER_ON);
  rfm12Cmd(0x0000);
}


void rfm12_startSleep(uint8_t n) {
  rfm12Cmd(RF_WAKEUP_TIMER | 0x0A00 | n);
  rfm12Cmd(RF_SLEEP_MODE);
  if (n > 0) {
    rfm12Cmd(RF_WAKEUP_MODE);
   }
   rfm12_state = RFM12_SLEEPING;
   rfm12_clearPins();
}


void rfm12_interrupt(void) {
  if(RFM12_INT_READ() == 0) {
    if(rfm12_state == RFM12_SLEEPING) {
      rfm12_setupPins();
      rfm12_setIdle();
    }
  }
}
