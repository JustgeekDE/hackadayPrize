/*
  RFM 12 Test program, sends out a short signal

 */
#include <stdint.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include "rfm12.h"
#include "sensor.h"


// Maximum number of skipped sends
#define MAX_SKIPS 2
#define NODE_ID 11
#define UPDATE_INTERVALL (140 + (NODE_ID>> 2))

uint8_t send_buffer[] = "ggt1t2t3sc";
uint16_t gap,temp1,temp2,temp3,foo;
uint32_t messageCounter = 0;
uint8_t skipCounter = 0;
uint8_t temp_ddr, temp_port;


static inline void setClockHigh(void){
	// 8 MHz
  CLKPR = (1<< CLKPCE);
  CLKPR = 0;
}

static inline void setClockLow(void){

	// 1 MHz
  CLKPR = (1<< CLKPCE);
//  CLKPR = (1 << CLKPS1) | (1 << CLKPS0);

	// 250 kHz
  CLKPR = (1 << CLKPS2) | (1 << CLKPS0);
}

static inline void setClockVeryLow(void){
	// 32 kHz
  CLKPR = (1<< CLKPCE);
  CLKPR = (1 << CLKPS3);
}


ISR(INT_VECT) {
  rfm12_interrupt();
}


void sleepNow(uint8_t delay) {
  rfm12_startSleep(delay);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sei();
  sleep_mode();
  cli();
  sleep_disable();

}

void startADC(void){
  PRR = ~(1<<PRADC);  //Disable everything except ADC
//  ADCSRA |= (1 << ADEN | 1 << ADPS2 | 1 << ADPS1);  //enable ADC, prescaler 64
//  ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0);  //enable ADC, prescaler 8
  ADCSRA = (1 << ADEN);  //enable ADC, prescaler 2
}

void stopADC(void){
  ADCSRA = ADCSRA & ~(1 << ADEN);  //disable ADC
  PRR = 0;  //Disable everything
}

void setup(void) {
  wdt_disable();
  cli();
  PRR = 0;  //Disable everything
  PORTB = 0xFF; //Pull up on everything
  skipCounter = 0;
  rfm12_init(NODE_ID, RFM12_868MHZ);

#ifdef PC_INT
  // Activate interupt on pin change
  PCMSK |= ( 1 << PCINT1);
  GIMSK |= ( 1 << PCIE);
#else
  // low level int0 generates interrupt
  MCUCR &= ~(( 1 << ISC01) | ( 1 << ISC00));
  GIMSK |= ( 1 << INT0);
#endif

  cli();
}


uint16_t adcRead( uint8_t channel )
{
  while (ADCSRA & (1<<ADSC) ) __asm volatile ("nop"::);
  // Select channel
  ADMUX = (ADMUX & ~(0x0F)) | (channel & 0x0F);

  //Measure
  ADCSRA |= (1<<ADSC);
  while (ADCSRA & (1<<ADSC) ) __asm volatile ("nop"::);
  return ADCW;
}

uint8_t readSensors(void ) {
  uint8_t change = 0;
  temp_ddr  = SENSOR_DDR;
  temp_port = SENSOR_PORT;
  startADC();

#ifdef SENSOR_SUPPLY
  SENSOR_VCC_OUT();
  SENSOR_VCC_LOW();
#endif
  SENSOR_1_IN();
  SENSOR_1_LOW();
  SENSOR_2_IN();
  SENSOR_2_LOW();

  ADMUX &=~(1 << REFS0 | 1 << REFS2); // 1.1 reference
  ADMUX |= (1 << REFS1);
  // discard first measurement
  adcRead(SENSOR_INTERNAL_CHANNEL);
  foo = adcRead(SENSOR_INTERNAL_CHANNEL);
  if(foo != temp1) {
    temp1 = foo;
    change = 1;
  }

  ADMUX &=~(1 << REFS0 | 1 << REFS1 | 1 << REFS2); // VCC reference
  // discard first measurement
  adcRead(SENSOR_GAP_CHANNEL);
  foo = adcRead(SENSOR_GAP_CHANNEL);
  if(foo != gap) {
    gap = foo;
    change = 1;
  }

  foo = 1023 - adcRead(SENSOR_1_CHANNEL);
  if(foo != temp2) {
    temp2 = foo;
    change = 1;
  }

  foo = 1023 - adcRead(SENSOR_2_CHANNEL);
  if(foo != temp3) {
    temp3 = foo;
    change = 1;
  }

  stopADC();
  SENSOR_PORT = temp_port;
  SENSOR_DDR  = temp_ddr;
  return change;
}

static inline void loop(void) {
  setClockLow();
	readSensors();
  setClockHigh();

  send_buffer[0] = gap   >> 8;
  send_buffer[1] = gap   & 0xFF;
  send_buffer[2] = temp1 >> 8;
  send_buffer[3] = temp1 & 0xFF;
  send_buffer[4] = temp2 >> 8;
  send_buffer[5] = temp2 & 0xFF;
  send_buffer[6] = temp3 >> 8;
  send_buffer[7] = temp3 & 0xFF;
  send_buffer[8] = messageCounter >> 24;
  send_buffer[9] = (messageCounter << 16) & 0xFF;

  rfm12_sendBuffer(0, (uint8_t*) send_buffer, 10, RFM12_MESSAGE_TYPE_TEMP_DATA, 0);
  messageCounter++;

  sleepNow(UPDATE_INTERVALL);
}

int main(void) {
  setClockHigh();
  setup();
  rfm12_sendBuffer(0, (uint8_t*) "Hi world", 8, RFM12_MESSAGE_TYPE_PLAIN, 0);

  while(1) {
    loop();
  }

  return 1;

}