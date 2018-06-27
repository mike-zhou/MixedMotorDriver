#
#	Module:			makefile
#
#					Copyright (C) Altera Corporation 1997-1999
#
#	Description:	Makefile for JAM Interpreter
#

CROSS_COMPILE=/home/user1/Tool/4.1.2/bin/arm-embedded-linux-gnu-
CFLAGS= -g -mcpu=arm920t
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

OBJS = \
	jamstub.o \
	jamexec.o \
	jamnote.o \
	jamcrc.o \
	jamsym.o \
	jamstack.o \
	jamheap.o \
	jamarray.o \
	jamcomp.o \
	jamjtag.o \
	jamutil.o \
	jamexp.o \
	stub_implement_9200.o

# LINK: add appropriate linker command here
all: jam.elf

jam.elf : $(OBJS)
	$(LD) -static -o jam.elf $(OBJS)

# Dependencies:

jamstub.o: jamstub.c jamport.h jamexprt.h stub_implement.h
	$(CC) $(CFLAGS) -c $<
	  
jamexec.o : \
	jamexec.c \
	jamport.h \
	jamexprt.h \
	jamdefs.h \
	jamexec.h \
	jamutil.h \
	jamexp.h \
	jamsym.h \
	jamstack.h \
	jamheap.h \
	jamarray.h \
	jamjtag.h
	$(CC) $(CFLAGS) -c $<

jamnote.o : \
	jamnote.c \
	jamexprt.h \
	jamdefs.h \
	jamexec.h \
	jamutil.h
	$(CC) $(CFLAGS) -c $<

jamcrc.o : \
	jamcrc.c \
	jamexprt.h \
	jamdefs.h \
	jamexec.h \
	jamutil.h
	$(CC) $(CFLAGS) -c $<

jamsym.o : \
	jamsym.c \
	jamexprt.h \
	jamdefs.h \
	jamsym.h \
	jamheap.h \
	jamutil.h
	$(CC) $(CFLAGS) -c $<

jamstack.o : \
	jamstack.c \
	jamexprt.h \
	jamdefs.h \
	jamutil.h \
	jamsym.h \
	jamstack.h
	$(CC) $(CFLAGS) -c $<

jamheap.o : \
	jamheap.c \
	jamport.h \
	jamexprt.h \
	jamdefs.h \
	jamsym.h \
	jamstack.h \
	jamheap.h \
	jamutil.h
	$(CC) $(CFLAGS) -c $<

jamarray.o : \
	jamarray.c \
	jamexprt.h \
	jamdefs.h \
	jamexec.h \
	jamexp.h \
	jamsym.h \
	jamstack.h \
	jamheap.h \
	jamutil.h \
	jamcomp.h \
	jamarray.h
	$(CC) $(CFLAGS) -c $<

jamcomp.o : \
	jamcomp.c \
	jamdefs.h \
	jamcomp.h
	$(CC) $(CFLAGS) -c $<

jamjtag.o : \
	jamjtag.c \
	jamexprt.h \
	jamdefs.h \
	jamsym.h \
	jamutil.h \
	jamjtag.h
	$(CC) $(CFLAGS) -c $<

jamutil.o : \
	jamutil.c \
	jamutil.h
	$(CC) $(CFLAGS) -c $<

jamexp.o : \
	jamexp.c \
	jamexprt.h \
	jamdefs.h \
	jamexp.h \
	jamsym.h \
	jamheap.h \
	jamarray.h \
	jamutil.h \
	jamytab.h
	$(CC) $(CFLAGS) -c $<
	
stub_implement_9200.o: \
    stub_implement_9200.c \
    stub_implement.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f jam.* *.o *~
