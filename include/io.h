#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>


#define MAX_DEVS 32 // should be WAY more than enough for our purposes
#define MAX_DEV_NAME_LEN 32

typedef struct io_dev_ops {
	uint8_t (*read)(uint16_t addr, void * priv_data);
	void (*write)(uint16_t addr, uint8_t val, void * priv_data);
	void (*reset)(void * priv_data);

} io_dev_ops_t;

typedef struct io_dev {
	char name[MAX_DEV_NAME_LEN];
	void * priv_data;
	io_dev_ops_t * ops;

	struct io_subsys * io; // parent pointer

	uint16_t block_start; // address of first block owned by this device
	size_t block_size; // how much IO memory it owns
	int slot_num; // slot on the I/O bus
} io_dev_t;

typedef struct io_page {
	io_dev_t * owner;
} io_page_t;

typedef struct io_subsys {
	io_dev_t * devs[MAX_DEVS];
} io_subsys_t;

io_subsys_t * io_init(struct system * sys);
void io_reset(io_subsys_t * io);

uint8_t io_read(uint16_t addr);
void io_write(uint16_t addr, uint8_t val);

int io_dev_create(io_subsys_t * io,
		  const char * name,
		  io_dev_ops_t * ops,
		  uint16_t io_addr,
		  size_t size,
		  void * priv_data);
void io_dev_destroy(io_dev_t * dev);

#endif /* !__IO_H__! */
