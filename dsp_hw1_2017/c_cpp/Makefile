.PHONY: all clean

CC = gcc 
CFLAGS+= -Wall -DDEBUG
LDFLAGS+=-lm     # link to math library

TARGET=train test

all: train test

train: train.c hmm.h
	$(CC) $< -o $@ $(CFLAGS)
# type make/make all to compile test_hmm

test: test.c hmm.h
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm $(TARGET)
#$(RM) $(TARGET)   # type make clean to remove the compiled file
