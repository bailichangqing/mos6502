#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <base.h>
#include <sys.h>
#include <io.h>
#include <dev/controller.h>
#include <gui/gui.h>
#include <mos6502/cpu.h>


#include <SDL.h>
#include <SDL2/SDL.h>


static uint8_t
handle_cont_port_read (cont_state_t * state, uint8_t portnum) 
{
	uint8_t val;
	switch (portnum) {
		case CONT_BUTTON_PORT:
			DEBUG_PRINT("Controller button port read\n");
			val = state->regs[CONT_BUTTON_PORT];
			state->regs[CONT_BUTTON_PORT] = 0;
			return val;
		default:
			return 0;
	}
			
		
	return 0;
}

static uint8_t 
cont_read (uint16_t addr, void * priv_data)
{
	cont_state_t * state = (cont_state_t*)priv_data;
	int regnum = CONT_REG_OFFSET(addr);
	if (regnum >= CONT_NUM_REGS) {
		return 0;
	} else {
		DEBUG_PRINT("Read from controller port 0x%02X (returning 0x%02X\n", regnum, state->regs[regnum]);
		return handle_cont_port_read(state, regnum);
	}

	return 0;
}

static void
cont_write (uint16_t addr, uint8_t val, void * priv_data)
{
	cont_state_t * state = (cont_state_t*)priv_data;
	int regnum = CONT_REG_OFFSET(addr);
	if (regnum < CONT_NUM_REGS) {
		DEBUG_PRINT("Write to controller port 0x%02X (writing 0x%02X)\n", regnum, val);
		state->regs[regnum] = val;
	}
}

static void
cont_reset (void * priv_data)
{
	DEBUG_PRINT("Controller reset\n");
}

static void
handle_a (controller_t * c)
{
	DEBUG_PRINT("%s\n", __func__);
	c->state->regs[CONT_BUTTON_PORT] = CONT_BUTTON_A;
}

static void
handle_b (controller_t * c)
{
	DEBUG_PRINT("%s\n", __func__);
	c->state->regs[CONT_BUTTON_PORT] = CONT_BUTTON_B;
}

static void
handle_up (controller_t * c)
{
	DEBUG_PRINT("%s\n", __func__);
	c->state->regs[CONT_BUTTON_PORT] = CONT_BUTTON_UP;
}

static void
handle_down (controller_t * c)
{
	DEBUG_PRINT("%s\n", __func__);
	c->state->regs[CONT_BUTTON_PORT] = CONT_BUTTON_DOWN;
}

static void
handle_left (controller_t * c)
{
	DEBUG_PRINT("%s\n", __func__);
	c->state->regs[CONT_BUTTON_PORT] = CONT_BUTTON_LEFT;
}

static void
handle_right (controller_t * c)
{
	DEBUG_PRINT("%s\n", __func__);
	c->state->regs[CONT_BUTTON_PORT] = CONT_BUTTON_RIGHT;
}

static void
handle_start (controller_t * c)
{
	DEBUG_PRINT("%s\n", __func__);
	c->state->regs[CONT_BUTTON_PORT] = CONT_BUTTON_STRT;
}

static void
handle_select (controller_t * c)
{
	DEBUG_PRINT("%s\n", __func__);
	c->state->regs[CONT_BUTTON_PORT] = CONT_BUTTON_SEL;
}


static void
handle_keypress (SDL_Keycode k, void * priv_data)
{
	controller_t * c = (controller_t *)priv_data;
	int own_key = 1;

	switch (k) {
		case SDLK_a:
			handle_a(c);
			break;
		case SDLK_b:
			handle_b(c);
			break;
		case SDLK_UP:
			handle_up(c);
			break;
		case SDLK_DOWN:
			handle_down(c);
			break;
		case SDLK_RIGHT:
			handle_right(c);
			break;
		case SDLK_LEFT:
			handle_left(c);
			break;
		case SDLK_s:
			handle_start(c);
			break;
		case SDLK_l:
			handle_select(c);
			break;
		default:
			own_key = 0;
			break;
	}

	if (own_key) {
		mos6502_raise_irq(c->sys->cpu);
	}
}


static io_dev_ops_t cont_ops = {
	.read  = cont_read,
	.write = cont_write,
	.reset = cont_reset
};

controller_t * 
controller_init (system_t * sys)
{
	INFO_PRINT("Initializing controller...\n");

	cont_state_t * state = malloc(sizeof(cont_state_t));
	if (!state) {
		ERROR_PRINT("Could not allocate controller state\n");
		return NULL;
	}
	memset(state, 0, sizeof(cont_state_t));

	controller_t * cont = malloc(sizeof(controller_t));

	if (!cont) {
		ERROR_PRINT("Could not allocate controller\n");
		goto out_err;
	}
	memset(cont, 0, sizeof(controller_t));

	cont->state = state;
	cont->sys   = sys;

	if (io_dev_create(sys->io,
		          "Controller",
			  &cont_ops,
			  CONT_MMIO_START,
			  CONT_MMIO_SIZE,
			  cont->state) != 0) {
		ERROR_PRINT("Could not create controller device\n");
		goto out_err1;
	}

	if (gui_register_keyhandler(sys->gui, 
				handle_keypress,
				(void*)cont) != 0) {
		ERROR_PRINT("Could not register key handler\n");
		goto out_err1;
	}
		

	return cont;
	
out_err1:
	free(cont);
out_err:
	free(state);
	return NULL;
}

