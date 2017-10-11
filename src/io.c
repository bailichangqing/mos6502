
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <base.h>
#include <sys.h>
#include <io.h>


// KCH: this should work out to 32 pages (page size is 256B)
static io_page_t io_pages[NPAGES_IO];

io_subsys_t * 
io_init (system_t * sys)
{

	io_subsys_t * io = NULL;

	INFO_PRINT("Initializing I/O subsystem...\n");

	io = malloc(sizeof(io_subsys_t));
	if (!io) {
		ERROR_PRINT("Could not allocate I/O subsystem\n");
		return NULL;
	}

	memset(io, 0, sizeof(io_subsys_t));
	
	memset(io_pages, 0, sizeof(io_page_t)*NPAGES_IO);

	return io;
}


int
io_dev_create (io_subsys_t * io,
	       const char * name,
	       io_dev_ops_t * ops,
	       uint16_t io_addr,
	       size_t size,
	       void * priv_data)
{
	int i, npages;
	uint8_t dev_slot_avail = 0;

	// allocate the device
	io_dev_t * dev = malloc(sizeof(io_dev_t));
	if (!dev) {
		ERROR_PRINT("Could not allocate device\n");
		return -1;
	}
	memset(dev, 0, sizeof(io_dev_t));

	// find a slot
	for (i = 0; i < MAX_DEVS; i++) {
		if (!io->devs[i]) {
			dev_slot_avail = 1;
			dev->slot_num  = i;
			io->devs[i] = dev;
			break;
		}
	}

	if (!dev_slot_avail) {
		ERROR_PRINT("Could not assign I/O device. No available slots\n");
		return -1;
	}

	dev->ops         = ops;
	dev->priv_data   = priv_data;
	dev->block_start = io_addr;
	dev->block_size  = size;
	dev->io          = io;

	if (name) {
		strncpy(dev->name, name, MAX_DEV_NAME_LEN);
	} else {
		sprintf(dev->name, "[Unknown Device]");
	}


	if (dev->block_start % PAGE_SIZE) {
		ERROR_PRINT("Bad I/O range (0x%x)\n", dev->block_start);
		goto out_err;
	}

	npages = dev->block_size >> PAGE_SHIFT;

	// assign the appropriate pages in I/O space to this device
	for (i = 0; i < npages; i++) {
		int pnum = ((dev->block_start-IO_START) >> PAGE_SHIFT) + i;
		io_pages[pnum].owner = dev;
	}

	INFO_PRINT("Created new device (%s) on slot %d. Start I/O addr=0x%x, size=0x%lx\n",
			dev->name,
			dev->slot_num,
			dev->block_start,
			dev->block_size);

	return 0;

out_err:
	free(dev);
	return -1;
}


void
io_dev_destroy (io_dev_t * dev)
{
	int npages, i;

	// free the bus slot
	dev->io->devs[dev->slot_num] = NULL;

	npages = dev->block_size >> PAGE_SHIFT;
	
	// abandon its I/O pages
	for (i = 0; i < npages; i++) {
		int pnum = ((dev->block_start-IO_START) >> PAGE_SHIFT) + i;
		io_pages[pnum].owner = NULL;
	}

	free(dev);
}


uint8_t
io_read (uint16_t addr)
{
	unsigned io_page_num = (addr - IO_START) >> PAGE_SHIFT;

	if (!io_pages[io_page_num].owner) {
		DEBUG_PRINT("Read to unhandled I/O address (0x%x). Returning 0\n", addr);
		return 0;
	} else {
		io_dev_t * owner = io_pages[io_page_num].owner;
		if (owner->ops && owner->ops->read) {
			return owner->ops->read(addr, owner->priv_data);
		} else {
			DEBUG_PRINT("Device did not supply read function. Returning 0\n");
			return 0;
		}
	}

	return 0;
}

void
io_write (uint16_t addr, uint8_t val)
{
	unsigned io_page_num = (addr - IO_START) >> PAGE_SHIFT;

	if (!io_pages[io_page_num].owner) {
		DEBUG_PRINT("Write to unhandled I/O address (0x%x)\n", addr);
	} else {
		io_dev_t * owner = io_pages[io_page_num].owner;
		if (owner->ops && owner->ops->write) {
			owner->ops->write(addr, val, owner->priv_data);
		} else {
			DEBUG_PRINT("Device did not supply write function.\n");
		}
	}
}

