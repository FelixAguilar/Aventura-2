#==============================================================================
# MAKEFILE FOR ALL LEVELS
# AUTHOR: AGUILAR, BENNASAR, BUENO
#==============================================================================

CC=gcc
CFLAGS=-c -g -Wall -std=c99
LDFLAGS=-lreadline

SOURCES= my_shell.c nivel7.c nivel6.c nivel5.c nivel4.c nivel3.c nivel2.c nivel1.c
LIBRARIES= #.o
INCLUDES= #.h
PROGRAMS= my_shell nivel7 nivel6 nivel5 nivel4 nivel3 nivel2 nivel1
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#   $(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

my_shell: my_shell.o 
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

nivel7: nivel7.o
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

nivel6: nivel6.o
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

nivel5: nivel5.o
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

nivel4: nivel4.o
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

nivel3: nivel3.o
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

nivel2: nivel2.o
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

nivel1: nivel1.o
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ *.tmp $(PROGRAMS)
