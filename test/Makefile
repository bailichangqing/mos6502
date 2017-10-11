
BASEDIR:=..
CFGDIR:=$(BASEDIR)/cfg
AS:=ca65
CC:=cc65
CL:=cl65
LD:=ld65
LDFLAGS:= -C $(CFGDIR)/hawknest.cfg 
CFLAGS:=
AFLAGS:=

LIBDIR:=$(BASEDIR)/lib
LIB:=hawknest.lib

all: testsuite

OBJS:=$(patsubst %.s,%.o, $(wildcard *.s))
OBJS+=$(patsubst %.c,%.o, $(wildcard *.c))

testsuite: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBDIR)/$(LIB)

test.o: test.c
	$(CC) $(CFLAGS) -o $@ $< 
	$(AS) $(AFLAGS) $@

testwrite.o: testwrite.s
	$(AS) $(AFLAGS) $<

clean:
	rm -f testwrite $(OBJS) *.bin testsuite

.PHONY: all clean
