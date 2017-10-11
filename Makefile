CC := gcc
LDFLAGS := 
CFLAGS := -Wall -Wno-unused-function -Iinclude -I/usr/include/SDL2 -O2
SRC := 
LIBS := -lSDL2 -lSDL2_ttf -lm -lrt

TARGET := hawknest

include .config

CFLAGS += -DDEBUG_ENABLE=$(DEBUG_ENABLE)

include src/modules.mk

OBJ := $(patsubst %.c, %.o, \
	 $(filter %.c, $(SRC)))

DEPENDS := $(OBJ:.o=.d)

$(TARGET): $(OBJ)
	@echo "Linking...[$(TARGET) <- $<]"
	@$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

all: $(TARGET)

clean:
	@rm -f $(OBJ) $(TARGET) $(DEPENDS)

include $(OBJ:.o=.d)

%.o: %.c
	@echo "$@ <- $<"
	@$(CC) $(CFLAGS) -c -o $@ $<
	
%.d: %.c
	@./depend.sh `dirname $*.c` \
	$(CFLAGS) $*.c > $@


.PHONY: all clean
