#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__


#define CONT_MMIO_START 0x8000 // right at the beginning
#define CONT_MMIO_SIZE  256 // one page
#define CONT_REG_OFFSET(x) ((x) - CONT_MMIO_START)

#define CONT_BUTTON_A     0x01
#define CONT_BUTTON_B     0x02
#define CONT_BUTTON_UP 	  0x03
#define CONT_BUTTON_DOWN  0x04
#define CONT_BUTTON_LEFT  0x05
#define CONT_BUTTON_RIGHT 0x06
#define CONT_BUTTON_STRT  0x07
#define CONT_BUTTON_SEL   0x08

/* reg 0 is button code */

#define CONT_NUM_REGS 8

#define CONT_BUTTON_PORT 0

typedef struct cont_state {
	uint8_t regs[CONT_NUM_REGS];
} cont_state_t;

typedef struct controller {
	cont_state_t * state;
	struct system * sys;
} controller_t; 


controller_t * controller_init(system_t * sys);


#endif /* !__CONTROLLER_H__! */
