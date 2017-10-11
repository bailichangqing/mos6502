#ifndef __VMCALL_H__
#define __VMCALL_H__

#define VMCALL_ARGS  0
#define VMCALL_EXIT  1 
#define VMCALL_OPEN  2
#define VMCALL_CLOSE 3
#define VMCALL_READ  4
#define VMCALL_WRITE 5

int handle_vmcall (decode_info_t * info);

int vmcall_init (int argc, char ** argv, int gui);

#endif
