#ifndef __PINS_H
#define __PINS_H


#define SET_BIT(port,bit)   (port |=  (1 << bit))
#define CLEAR_BIT(port,bit) (port &= ~(1 << bit))
#define READ_BIT(port,bit)  ((port & (1 << bit)) ? 1 : 0)

#endif