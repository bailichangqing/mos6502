#ifndef __GFX_H__
#define __GFX_H__


#define GFX_MMIO_START 0x9000
#define GFX_MMIO_SIZE  0x3000
#define GFX_FB_START   0x9000
#define GFX_FB_SIZE    0xFA0
#define GFX_CMD_PORT   (GFX_FB_START+GFX_FB_SIZE)



#define GFX_CGA_WIDTH  80
#define GFX_CGA_HEIGHT 25


typedef enum cga_cmd {
	CMD_SET_CURSOR = 0,
} cga_cmd_t;

typedef enum cga_color {
	COLOR_BLACK    = 0,
	COLOR_BLUE     = 1,
	COLOR_GREEN    = 2,
	COLOR_CYAN     = 3,
	COLOR_RED      = 4,
	COLOR_MAGENTA  = 5,
	COLOR_BROWN    = 6,
	COLOR_LGREY    = 7,
	COLOR_DGREY    = 8,
	COLOR_LBLUE    = 9,
	COLOR_LGREEN   = 10,
	COLOR_LCYAN    = 11,
	COLOR_LRED     = 12,
	COLOR_LMAGENTA = 13,
	COLOR_LBROWN   = 14,
	COLOR_WHITE    = 15
} cga_color_t;

typedef struct gfx_state {
	uint8_t fb[GFX_FB_SIZE];
} gfx_state_t;

typedef struct gfx_card {
	gfx_state_t * state;
	struct system * sys;
} gfx_card_t;

gfx_card_t * gfx_init(struct system * sys);

#endif /* !__GFX_H__! */
