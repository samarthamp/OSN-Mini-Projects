CC = gcc
CFLAGS = -Wall -g

OBJS = main.o prompt-1.o command-2.o hop-3.o reveal-4.o log-5.o proclore-7.o seek-8.o myshrc-9.o ioredir-10.o pipes-11.o activities-13.o signals-14.o fgbg-15.o neonate-16.o iman-17.o

TARGET = myshell

LIBS = -lssl -lcrypto

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

main.o: main.c header.h prompt-1.h command-2.h log-5.h myshrc-9.h activities-13.h signals-14.h
	$(CC) $(CFLAGS) -c main.c

prompt-1.o: prompt-1.c header.h prompt-1.h
	$(CC) $(CFLAGS) -c prompt-1.c

command-2.o: command-2.c header.h command-2.h hop-3.h reveal-4.h log-5.h proclore-7.h seek-8.h myshrc-9.h pipes-11.h activities-13.h signals-14.h fgbg-15.h
	$(CC) $(CFLAGS) -c command-2.c

hop-3.o: hop-3.c header.h hop-3.h
	$(CC) $(CFLAGS) -c hop-3.c

reveal-4.o: reveal-4.c header.h reveal-4.h
	$(CC) $(CFLAGS) -c reveal-4.c

log-5.o: log-5.c header.h log-5.h command-2.h
	$(CC) $(CFLAGS) -c log-5.c

proclore-7.o: proclore-7.c header.h proclore-7.h
	$(CC) $(CFLAGS) -c proclore-7.c

seek-8.o: seek-8.c header.h seek-8.h
	$(CC) $(CFLAGS) -c seek-8.c

myshrc-9.o: myshrc-9.c header.h myshrc-9.h command-2.h
	$(CC) $(CFLAGS) -c myshrc-9.c

ioredir-10.o: ioredir-10.c header.h ioredir-10.h
	$(CC) $(CFLAGS) -c ioredir-10.c

pipes-11.o: pipes-11.c header.h pipes-11.h ioredir-10.h command-2.h activities-13.h signals-14.h
	$(CC) $(CFLAGS) -c pipes-11.c

activities-13.o: activities-13.c header.h activities-13.h
	$(CC) $(CFLAGS) -c activities-13.c

signals-14.o: signals-14.c header.h signals-14.h activities-13.h
	$(CC) $(CFLAGS) -c signals-14.c

fgbg-15.o: fgbg-15.c header.h fgbg-15.h activities-13.h signals-14.h
	$(CC) $(CFLAGS) -c fgbg-15.c

neonate-16.o: neonate-16.c header.h neonate-16.h
	$(CC) $(CFLAGS) -c neonate-16.c

iman-17.o: iman-17.c header.h iman-17.h
	$(CC) $(CFLAGS) -c iman-17.c	

clean:
	rm -f *.o $(TARGET)