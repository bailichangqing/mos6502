#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <base.h>
#include <sys.h>
#include <io.h>

#include <dev/gfx.h>

#include <gui/gui.h>


static uint8_t 
fb_read (uint16_t addr, void * priv_data)
{
	gfx_card_t * gfx = (gfx_card_t*)priv_data;
	
	if (addr > GFX_FB_START+GFX_FB_SIZE) {
		ERROR_PRINT("Invalid framebuffer read from addr 0x%04X\n", addr);
		return 0;
	}

	INFO_PRINT("FB Read (0x%04X)\n", addr);
	
	return gfx->state->fb[addr - GFX_FB_START];
}

static void
fb_write (uint16_t addr, uint8_t val, void * priv_data)
{
	gfx_card_t * gfx = (gfx_card_t*)priv_data;
	uint8_t oldval;

	if (addr > GFX_FB_START+GFX_FB_SIZE) {
		ERROR_PRINT("Invalid framebuffer write to addr 0x%04X\n", addr);
		return;
	}

	INFO_PRINT("FB Write (0x%04X = 0x%02X)\n", addr, val);

	oldval = gfx->state->fb[addr - GFX_FB_START];
	gfx->state->fb[addr - GFX_FB_START] = val;

	// send it over to the GUI
	if (oldval != val) {
		gui_update_console_char(gfx->sys,
					val,
					(addr-GFX_FB_START) % GFX_CGA_WIDTH,
					(addr-GFX_FB_START) / GFX_CGA_WIDTH);
		
	}
}

static void
fb_reset (void * priv_data)
{
	gfx_card_t * gfx = (gfx_card_t*)priv_data;
	memset(gfx->state->fb, ' ', GFX_FB_SIZE);
}

static io_dev_ops_t fb_ops = {
	.read  = fb_read,
	.write = fb_write,
	.reset = fb_reset,
};

gfx_card_t *
gfx_init (system_t * sys)
{
	gfx_card_t * gfx = NULL;

	gfx = malloc(sizeof(gfx_card_t));
	if (!gfx) {
		ERROR_PRINT("Could not allocate graphics card\n");
		return NULL;
	}

	memset(gfx, 0, sizeof(gfx_card_t));
	
	gfx->state = malloc(sizeof(gfx_state_t));
	if (!gfx->state) {
		ERROR_PRINT("Could not allocate graphics state\n");
		goto out_err;
	}

	memset(gfx->state->fb, ' ', GFX_FB_SIZE);

	gfx->sys = sys;

	if (io_dev_create(sys->io,
			  "FB Console",
			   &fb_ops,
			   GFX_MMIO_START,
			   GFX_MMIO_SIZE,
			   gfx) != 0) {
		ERROR_PRINT("Could not create gfx device\n");	
		goto out_err1;
	}
			   
	return gfx;

out_err1:
	free(gfx->state);
out_err:
	free(gfx);
	return NULL;

}


