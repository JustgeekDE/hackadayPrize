#include <stdint.h>

#include "light.h"
#include "effects.h"

volatile uint16_t pwm_time = 0;

effectData effects_queue[EFFECTS_QUEUE_LENGTH];
uint8_t effects_readIndex;
uint8_t effects_writeIndex;
uint8_t effects_seed = 42;;

// this could also be done with a a cast etc. but i'm not quite sure yet, wether i want to use LE or BE
inline uint16_t effects_fromLE(uint8_t first, uint8_t second) {
  return (first + (second << 8));
}

// simple LCG because rand() is to big
inline uint8_t effects_rand(void) {
//  return 250;
  return effects_seed = ((effects_seed * EFFECTS_LCG_FACTOR) + 1) & 0xFF;
}

void effects_setup(void) {
  uint8_t i;
  for (i = 0; i < EFFECTS_QUEUE_LENGTH; i++){
    effects_queue[i].type = EFFECT_EMPTY;
  }
  effects_readIndex = 0;
  effects_writeIndex = 0;
}

inline void effects_checkNext(void) {
  effectData* nextEffect;
  nextEffect = &effects_queue[(effects_readIndex + 1) % EFFECTS_QUEUE_LENGTH];
  if ((nextEffect->type != EFFECT_EMPTY) && (nextEffect->start <= pwm_getTime())) {
    // clear current effect and move on
    effects_queue[effects_readIndex].type = EFFECT_EMPTY;
    effects_readIndex = (effects_readIndex + 1) % EFFECTS_QUEUE_LENGTH;
  }
}


uint8_t effects_insertEffect(uint8_t* data, uint8_t length) {
  if(effects_queue[effects_writeIndex].type != EFFECT_EMPTY) {
    return 0;
  }
  uint8_t i;

  effects_queue[effects_writeIndex].type = data[0];
  effects_queue[effects_writeIndex].start = pwm_getTime() + effects_fromLE(data[1], data[2]);

  // Blank out scratchpad
  for(i = 0; i < EFFECTS_SCRATCH_SIZE; i++) {
    effects_queue[effects_writeIndex].scratchPad[i] = 0;
  }

  // Copy data
  for(i = 0; (i < (length-3)) && (i < EFFECTS_SCRATCH_SIZE); i++) {
    effects_queue[effects_writeIndex].scratchPad[i] = data[i+3];
  }
  effects_writeIndex++;
  while(effects_writeIndex >= EFFECTS_QUEUE_LENGTH) {
    effects_writeIndex -= EFFECTS_QUEUE_LENGTH;
  }
  return 1;
}


uint8_t effects_calculateSingleFade(uint32_t currentPos, uint16_t length, uint8_t startValue, uint8_t endValue) {
  if(currentPos > length) {
    return endValue;
  }
  int32_t temp;

  temp = (((int32_t) endValue) - ((int32_t) startValue));
  temp *= (int32_t) currentPos;
  temp = temp / ((int32_t) length-1);
  temp += (int32_t) startValue;
  temp = temp > 255 ? 255 : temp < 0 ? 0 : temp;
  return temp;
}


void effects_loop(void) {
  effects_checkNext();
  effectData* effect = &effects_queue[effects_readIndex];
  uint8_t r,g,b;

  if(effect->start > pwm_getTime()) {
    return;
  }

  switch(effect->type) {
    case EFFECT_LIGHT:
      light_setValue(effect->scratchPad[0],effect->scratchPad[1],effect->scratchPad[2], 0);
      break;

    case EFFECT_LIGHT_LIN:
      light_setValue(effect->scratchPad[0],effect->scratchPad[1],effect->scratchPad[2], 1);
      break;

    case EFFECT_STROBE_FOFF:
    case EFFECT_STROBE: {
      uint16_t strobePeriod = effects_fromLE(effect->scratchPad[0], effect->scratchPad[1]);
      uint16_t onPeriod = effects_fromLE(effect->scratchPad[2], effect->scratchPad[3]);

      // reduce time to current period
      uint16_t currentPos = (pwm_getTime() - effect->start) % strobePeriod;
      if(currentPos < onPeriod) {
        r = effect->scratchPad[4];
        g = effect->scratchPad[5];
        b = effect->scratchPad[6];
      } else {
        if(effect->type == EFFECT_STROBE_FOFF) {
          currentPos   -= onPeriod;
          strobePeriod -= onPeriod;
          r = effects_calculateSingleFade(currentPos, strobePeriod > 1, effect->scratchPad[4], effect->scratchPad[7]);
          g = effects_calculateSingleFade(currentPos, strobePeriod > 1, effect->scratchPad[5], effect->scratchPad[8]);
          b = effects_calculateSingleFade(currentPos, strobePeriod > 1, effect->scratchPad[6], effect->scratchPad[9]);
        } else {
          r = effect->scratchPad[7];
          g = effect->scratchPad[8];
          b = effect->scratchPad[9];
        }
      }
      light_setValue(r,g,b,0);
      break;
    }

    case EFFECT_FADE_LIN:
    case EFFECT_FADE: {
      uint16_t fadeDuration = effects_fromLE(effect->scratchPad[0], effect->scratchPad[1]);
      uint32_t currentPos = pwm_getTime() - effect->start;
      r = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[2], effect->scratchPad[5]);
      g = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[3], effect->scratchPad[6]);
      b = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[4], effect->scratchPad[7]);

      light_setValue(r,g,b, (effect->type == EFFECT_FADE_LIN));
      break;
    }

    case EFFECT_FADE_CONT_LIN:
    case EFFECT_FADE_CONT: {
      uint16_t fadeDuration = effects_fromLE(effect->scratchPad[0], effect->scratchPad[1]);
      uint16_t currentPos = (pwm_getTime() - effect->start) % (fadeDuration * 2);


      if(currentPos > fadeDuration) {
        currentPos -= fadeDuration;
        //fade out
        r = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[5], effect->scratchPad[2]);
        g = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[6], effect->scratchPad[3]);
        b = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[7], effect->scratchPad[4]);
      } else {
        //fade in
        r = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[2], effect->scratchPad[5]);
        g = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[3], effect->scratchPad[6]);
        b = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[4], effect->scratchPad[7]);
      }
      light_setValue(r,g,b, (effect->type == EFFECT_FADE_CONT_LIN));
      break;
    }


    case EFFECT_FADE_CONT_RAND: {
      /* scratchpad usage:
       * [0]  duration
       * [1]  duration
       * [2]  r start
       * [3]  g start
       * [4]  b start
       * [5]  r end
       * [6]  g end
       * [7]  b end
       */


      uint16_t fadeDuration = effects_fromLE(effect->scratchPad[0], effect->scratchPad[1]);
      uint8_t switchColors = 0;
      uint16_t currentPos = (pwm_getTime() - effect->start);

      while (currentPos > fadeDuration) {
        effect->start += fadeDuration;
        currentPos = (pwm_getTime() - effect->start);
        switchColors = 1;
      }

      if(switchColors) {
        //switch target color to start color
        effect->scratchPad[2] = effect->scratchPad[5];
        effect->scratchPad[3] = effect->scratchPad[6];
        effect->scratchPad[4] = effect->scratchPad[7];

        // set new target color
        effect->scratchPad[5] = effects_rand();
        effect->scratchPad[6] = effects_rand();
        effect->scratchPad[7] = effects_rand();

        // scale colors to assure maximum brightness
        uint8_t max = effect->scratchPad[5] > effect->scratchPad[6] ? effect->scratchPad[5] : effect->scratchPad[6];
        max = effect->scratchPad[7] > max ? effect->scratchPad[7] : max;

        effect->scratchPad[5] = (effect->scratchPad[5] * 255) / max;
        effect->scratchPad[6] = (effect->scratchPad[6] * 255) / max;
        effect->scratchPad[7] = (effect->scratchPad[7] * 255) / max;

/*
        uint16_t temp = (127 * 3) - effect->scratchPad[5] - effect->scratchPad[6];
        effect->scratchPad[7] = temp > 255 ? 255 : temp;
*/
      }


      r = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[2], effect->scratchPad[5]);
      g = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[3], effect->scratchPad[6]);
      b = effects_calculateSingleFade(currentPos, fadeDuration, effect->scratchPad[4], effect->scratchPad[7]);
      light_setValue(r,g,b, 1);
      break;
    }

    case EFFECT_FLAME: {
      //uint8_t speed, uint8_t r,g,b, uint32_t next switch
      uint32_t* nextChange = ((uint32_t*) effect->scratchPad);
      if(nextChange[2] < pwm_getTime()) {
        uint16_t temp;

        // set time till next change
        temp = (effects_rand() > 1) * effect->scratchPad[0];
        nextChange[2] = pwm_getTime() + temp;

        // set colours
        temp = effects_rand();
        r = effects_calculateSingleFade(temp, 0xFF , effect->scratchPad[4], effect->scratchPad[1]);
        g = effects_calculateSingleFade(temp, 0xFF , effect->scratchPad[5], effect->scratchPad[2]);
        b = effects_calculateSingleFade(temp, 0xFF , effect->scratchPad[6], effect->scratchPad[3]);
        light_setValue(r,g,b, 1);
      }
      break;
    }

    default:
      break;
  }
}

