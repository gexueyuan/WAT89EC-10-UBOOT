.SUFFIXES : .c .o

CC		= gcc
ASM		=
LINK		=
LIBCC		= ar
RM		= rm

#----------------------------------------------------------------------------------------------
INCPATH		=
LIBPATH		= 
OBJPATH		= .
OUTPATH		= .
SOURCEPATH	= .

LDFLAGS		= -L$(LIBPATH) 
#LFLAGS		= -lm
ASFLAGS		=
ARFLAGS		= -ruv
CFLAGS		= -o

LIBS =

#---------------------------------------------------------------------------------------
SOURCES = $(OBJECTS:.o=.c)

all:
	$(CC) $(CFLAGS)	add_checksum add_checksum.c
	$(CC) $(CFLAGS)	size_adjust size_adjust2.c
	$(CC) $(CFLAGS)	sd_fdisk sd_fdisk.c

#---------------------------------------------------------------------------------------
.c.o:
		$(CC) $(CFLAGS) -c $< -o $@

dep:
		gccmakedep	$(SOURCES)

#---------------------------------------------------------------------------------------
clean:
		$(RM)		-rf sd_fdisk
		$(RM)		-rf size_adjust

new:
		$(MAKE)		clean
		$(MAKE)

