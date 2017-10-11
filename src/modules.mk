SRC += src/main.c \
       src/sys.c \
       src/memctrl.c \
       src/cartrom.c \
       src/sysrom.c \
       src/io.c \
       src/shell.c

include src/mos6502/modules.mk
include src/gui/modules.mk
include src/dev/modules.mk

