CC = g++

CFLAGS = -c --std=c++11

OBJDIR = obj

SRCDIR = src

SERVICEDIR = services

BINDIR = bin

compile : $(BINDIR)/server $(BINDIR)/client

$(BINDIR)/server : $(OBJDIR)/clientSocketQueue.o $(OBJDIR)/server1.o
	$(CC) $(OBJDIR)/clientSocketQueue.o $(OBJDIR)/server1.o -o $(BINDIR)/server

$(BINDIR)/client : $(OBJDIR)/client1.o
	$(CC) $(OBJDIR)/client1.o -o $(BINDIR)/client

$(OBJDIR)/server1.o : $(SRCDIR)/server1.cpp
	$(CC) $(CFLAGS) $(SRCDIR)/server1.cpp -o $(OBJDIR)/server1.o

$(OBJDIR)/client1.o : $(SRCDIR)/client1.cpp
	$(CC) $(CFLAGS) $(SRCDIR)/client1.cpp -o $(OBJDIR)/client1.o

$(OBJDIR)/clientSocketQueue.o : $(SERVICEDIR)/clientSocketQueue.cpp
	$(CC) $(CFLAGS) $(SERVICEDIR)/clientSocketQueue.cpp -o $(OBJDIR)/clientSocketQueue.o

clean : 
	rm $(OBJDIR)/* $(BINDIR)/server $(BINDIR)/client