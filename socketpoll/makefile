SERV = webServer

CLI = webClient


SRCS := Socket.cpp \
		webServer.cpp \
		webClient.cpp \
		ThreadPoll.cpp 

#patsubst 逗号之间不能存在空格,以%.o替换掉%.cpp
OBJS = $(patsubst %.cpp,%.o,$(SRCS))


CC = g++
CFLAGS = -g 
LDFLAGS = -I/usr/include -L/usr/lib -lncurses -lrt  -L .
CXXFLAGS := -w -std=c++11 -lpthread 

count := 1

SSRC = $(filter-out $(CLI).o,$(OBJS))
CSRC = $(filter-out $(SERV).o,$(OBJS))

all:$(SERV) $(CLI)

$(SERV):$(SSRC)
	$(CC)  $^ -o $@ $(CXXFLAGS)

$(CLI):$(CSRC)
	$(CC) $^ -o $@ $(CXXFLAGS)

#$(EXE):$(OBJS)
#	$(CC)  $^ -o $@ $(LDFLAGS)

$(OBJS):%.o:%.cpp 
	@echo $(OBJS) -------  ----------
	$(CC)  -c $(CFLAGS) $< -o $@ $(CXXFLAGS) 



.PHONY:clean
clean:
	-$(RM) $(OBJS) $(CLI) $(SERV)

