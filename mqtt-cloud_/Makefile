
#CC = arm-none-linux-gnueabi-gcc
CC = gcc 
CFLAGS = -I$(INCLUDE_DIR) -I$(LIB_INC) -g -Wall 
LFLAGS = -L$(LIB_DIR)  -lpaho-mqtt3as -lpaho-mqtt3a -lpthread -lrt #   -lpaho-mqtt3cs     -lpaho-mqtt3c -lssl

prefix ?= 

ASYNC_EXE = paho-pub

CUR_DIR = $(shell pwd)
BIN_DIR = $(CUR_DIR)/bin
OBJ_DIR = $(CUR_DIR)/obj
SRC_DIR = $(CUR_DIR)/src

SRC :=  $(wildcard ${SRC_DIR}/*.c)
OBJ  := $(patsubst %.c,${OBJ_DIR}/%.o,${notdir $(SRC)})
BIN  := $(notdir $(CURDIR))

ASYNC_OBJ = ${filter-out ${OBJ_DIR}/main.o,$(OBJ)}

#export $(OBJ) $(SRC) $(CFLAGS) $(LFLAGS) $(CC)

#SRC := $(wildcard *.c)
#OBJ := $(SRC:%.c=%.o)

all : MK_DIR $(BIN)

MK_DIR:
	@mkdir -p $(BIN_DIR) $(OBJ_DIR)
	@echo $(ASYNC_OBJ) 
	#@echo $(BIN) 
	@echo -----------------------------------

$(BIN):$(OBJ)
	$(CC) $(ASYNC_OBJ) -o $@ $(CFLAGS) $(LFLAGS)

#$(BIN):$(OBJ)
#	$(CC) $(OBJ) -o mqtt-imx $(CFLAGS) $(LFLAGS)

$(OBJ):$(SRC)
	#$(CC) -c $< -o $@ $(CFLAGS) $(LFLAGS)
	$(CC) -c ${SRC_DIR}/$(*F).c -o $@ $(CFLAGS)


clean :
	rm -rf $(BIN_DIR) $(OBJ_DIR) *.o $(BIN) $(ASYNC_EXE)


