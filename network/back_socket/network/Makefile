SERV = aserver

CLI = aclient


SRCS := server.c \
		client.c 

#patsubst 逗号之间不能存在空格,以%.o替换掉%.cpp
OBJS = $(patsubst %.c,%.o,$(SRCS))


CC = gcc
CFLAGS = -g 
#LDFLAGS = -I/usr/include -L/usr/lib -lncurses -lrt  -L .
CXXFLAGS := 
#-w -std=c++11 -lpthread 


SSRC = $(filter-out client.o,$(OBJS))
CSRC = $(filter-out server.o,$(OBJS))

all:$(SERV) $(CLI)

$(SERV):$(SSRC)
	$(CC)  $^ -o $@ $(CXXFLAGS)

$(CLI):$(CSRC)
	$(CC) $^ -o $@ $(CXXFLAGS)

#$(EXE):$(OBJS)
#	$(CC)  $^ -o $@ $(LDFLAGS)

$(OBJS):%.o:%.c
	@echo $(OBJS) -------  ----------
	$(CC)  -c $(CFLAGS) $< -o $@ $(CXXFLAGS) 



.PHONY:clean
clean:
	-$(RM) $(OBJS) $(CLI) $(SERV)

