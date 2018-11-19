##################################################
##############	UNIVERSAL makefile	##############
TARGET = binary
LIBS = -lm
CC = gcc
CFLAGS = #-g -Wall

SRC_DIR = ./src/
BUILD_DIR = ./bin/

.PHONY: default all clean run

default: $(TARGET)

# read all headers and sources form the SRC_DIR
HEADERS = $(wildcard $(SRC_DIR)*.h)
SOURCES = $(wildcard $(SRC_DIR)*.c)

# make all '.o' file names from '.c' files
OBJECTS = $(patsubst $(SRC_DIR)%.c, $(BUILD_DIR)%.o, $(SOURCES))

# compile each source file
$(BUILD_DIR)%.o: $(SRC_DIR)%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)


# the main compilation of all '.o' files
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LIBS) -o $(BUILD_DIR)$@




##################################################
### build
all: default

### remove all compiled files
clean:
	-rm -f $(BUILD_DIR)$(TARGET)
	-rm -f $(BUILD_DIR)*.o
### run
run:
	./bin/binary