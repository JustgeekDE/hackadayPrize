#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "pins.h"
#include "rfm12.h"
#include "light.h"
#include "effects.h"

#define LED_PORT PORTA
#define LED_DDR  DDRA
#define LED_BIT  PA1

#define ACK_MESSAGES 1

uint8_t tempBuffer[12];

/*
                        // type                | start  |  | period |  |   on   |  | r    g      b   | r    g     b |
uint8_t effect0[] = {EFFECT_STROBE_FOFF,       0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00};
                    // type                    | start  |  |  time  |  | r    g      b | | r    g     b |
uint8_t effect1[] = {EFFECT_FADE_DIR_CONT_LIN, 0x00, 0x40, 0x00, 0x04, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
                         // type               | start  |  | r    g     b |
//uint8_t effect2[] = {EFFECT_LIGHT,             0x05, 0x00, 0x00, 0xFF, 0x00};
*/

static inline void setStartEffect(void){
/*
  tempBuffer[0]  = EFFECT_FADE_DIR_CONT_LIN;
  tempBuffer[1]  = 0x00;
  tempBuffer[2]  = 0x00;
  tempBuffer[3]  = 0x00;
  tempBuffer[4]  = 0x04;
  tempBuffer[5]  = 0x00;
  tempBuffer[6]  = 0x00;
  tempBuffer[7]  = 0xFF;
  tempBuffer[8]  = 0x00;
  tempBuffer[9]  = 0xFF;
  tempBuffer[10] = 0x00;
  effects_insertEffect(tempBuffer, 11);

  tempBuffer[0]  = EFFECT_FLAME;
  tempBuffer[1]  = 0x00;
  tempBuffer[2]  = 0x00;
  tempBuffer[3]  = 0x1F;
  tempBuffer[4]  = 0xD0;
  tempBuffer[5]  = 0xA0;
  tempBuffer[6]  = 0x20;
  tempBuffer[7]  = 0x70;
  tempBuffer[8]  = 0x50;
  tempBuffer[9]  = 0x10;
  effects_insertEffect(tempBuffer, 10);
  tempBuffer[0]  = EFFECT_FADE_CONT_RAND;
  tempBuffer[1]  = 0x00;
  tempBuffer[2]  = 0x00;
  tempBuffer[3]  = 0x00;
  tempBuffer[4]  = 0x0F;
  tempBuffer[5]  = 0x00;
  tempBuffer[6]  = 0x00;
  tempBuffer[7]  = 0xFF;
  tempBuffer[8]  = 0x00;
  tempBuffer[9]  = 0xFF;
  tempBuffer[10] = 0x00;
  effects_insertEffect(tempBuffer, 11);
*/
  tempBuffer[0]  = EFFECT_LIGHT;
  tempBuffer[1]  = 0x00;  // start time
  tempBuffer[2]  = 0x00;  // end time
  tempBuffer[3]  = 0x88;  // red
  tempBuffer[4]  = 0xFF;  // green
  tempBuffer[5]  = 0x80;  // blue
  tempBuffer[6]  = 0xFF;
  tempBuffer[7]  = 0xFF;
  tempBuffer[8]  = 0xFF;
  tempBuffer[9]  = 0xFF;
  tempBuffer[10] = 0xFF;
  effects_insertEffect(tempBuffer, 11);


}

int main(void) {
  wdt_disable();
  cli();

  light_setup();
  effects_setup();
  setStartEffect();
  rfm12_init(RFM12_868MHZ);

  // Activate interupt on low level for rfm12
  GIMSK |= ( 1 << INT0);
  MCUCR &=~(1 << ISC00);
  MCUCR &=~(1 << ISC01);

  SET_BIT(LED_DDR, LED_BIT);
  sei();

  rfm12_disableInterrupt();
  rfm12_sendBlocking(0, "Hi!", 3, RFM12_MESSAGE_TYPE_PLAIN, 0);
  rfm12_enableInterrupt();
  rfm12_recieverOn();

  wdt_enable(WDTO_8S);
  while(1){
    wdt_reset();
    if(rfm12_getState() != RFM12_RECIEVING) {
      volatile rfm12_Message* recievedData = rfm12_getMessage();
      if(recievedData->errors == 0) {
        switch(recievedData->type) {
          case RFM12_MESSAGE_TYPE_RGB_LIN:
            tempBuffer[0] = EFFECT_LIGHT_LIN;
            tempBuffer[1] = 0;
            tempBuffer[2] = 0;
            tempBuffer[3] = recievedData->data[0];
            tempBuffer[4] = recievedData->data[1];
            tempBuffer[5] = recievedData->data[2];
            effects_insertEffect(tempBuffer, 10);
//            light_setValue(recievedData->data[0], recievedData->data[1], recievedData->data[2]);
            break;

          case RFM12_MESSAGE_TYPE_SET_NODE_ID:
            rfm12_saveNodeId(recievedData->data[0]);
            break;

          case RFM12_MESSAGE_TYPE_RGB_ANIM:
            effects_insertEffect(recievedData->data, recievedData->length);
            break;

          default:
            break;
        }

        if((recievedData->flags & RFM12_FLAG_REQ_ACK) && ACK_MESSAGES) {
          _delay_ms(1);
          rfm12_disableInterrupt();
          rfm12_replyAck();
          rfm12_enableInterrupt();
        }

      }
      rfm12_recieverOn();
    }
    effects_loop();
  }
  return 1;
}