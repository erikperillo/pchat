DEPS = group.h message.h user.h elementnotfound.h chat.h chatview.h net.h protocol.h hash.h
OBJS = group.o message.o user.o elementnotfound.o chat.o chatview.o net.o protocol.o hash.o
OBJSDIR = .
CC = g++
CCFLAGS = -std=c++14 -Wall -lpthread

all: client server

%.o: %.cpp $(DEPS)
	$(CC) -c -o $(OBJSDIR)/$@ $< $(CCFLAGS)

test: $(OBJS) test.cpp
	$(CC) -o test test.cpp $(OBJS) $(CLIFLAGS) $(CCFLAGS)

server: $(OBJS) server.cpp
	$(CC) -o server server.cpp $(OBJS) $(CLIFLAGS) $(CCFLAGS)

client: $(OBJS) client.cpp
	$(CC) -o client client.cpp $(OBJS) $(CLIFLAGS) $(CCFLAGS)
