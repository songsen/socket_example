
target = shmread shmwrite
shmread = $(filter-out  $(FILTER_MAIN),$(OBJ)) $(OBJ_DIR)/shmread.o
shmwrite = $(filter-out $(FILTER_MAIN),$(OBJ)) $(OBJ_DIR)/shmwrite.o

CC = gcc
CFLAG = -g -Wall
LFLAG = -lrt

prefix ?= .
OBJ_DIR = $(prefix)/obj
BIN_DIR = $(prefix)/bin 

SRCS = $(wildcard *.c)
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))
FILTER_MAIN = $(patsubst %,$(OBJ_DIR)/%.o,$(target))

all: MKDIR $(target)

MKDIR:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)
	@echo $(BIN_shmr) --1---
	@echo $(BIN_shmw) --2---
	@echo $(FILTER_MAIN)
	@echo $(OBJ)

$(target):$(OBJ)
	$(CC) $($@) -o $@  $(CFLAG) $(LFLAG)

$(OBJ):$(SRCS)
	$(CC) -c $(*F).c -o $@ $(CFLAG) $(LFLAG)

.PHY clean:
	rm -rf $(OBJ) $(target) $(OBJ_DIR) $(BIN_DIR)




