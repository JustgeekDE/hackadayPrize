#ifndef __EFFECTS_H
#define __EFFECTS_H
#include <stdint.h>

#define EFFECTS_QUEUE_LENGTH 2
#define EFFECTS_SCRATCH_SIZE 12
#define EFFECTS_LCG_FACTOR 35

typedef struct {
  uint32_t start;
  uint8_t scratchPad[EFFECTS_SCRATCH_SIZE];
  uint8_t type;
  uint8_t id;

} effectData;

#define EFFECT_EMPTY             0x00
#define EFFECT_LIGHT             0x10
#define EFFECT_LIGHT_LIN         0x11
#define EFFECT_STROBE            0x20
#define EFFECT_STROBE_FOFF       0x21
#define EFFECT_FADE              0x30
#define EFFECT_FADE_LIN          0x31
#define EFFECT_FADE_CONT         0x32
#define EFFECT_FADE_CONT_LIN     0x33
#define EFFECT_FADE_CONT_RAND    0x34
#define EFFECT_FLAME             0x40

extern void effects_setup(void);
extern uint8_t effects_insertEffect(uint8_t* data, uint8_t length);
extern void effects_loop(void);

#endif