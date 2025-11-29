CC = gcc
CFLAGS = -Wall -Wextra -g
OBJS = procman.o main.o

procman: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o procman

test: procman
	bash test_scripts/test1.sh
	bash test_scripts/test2.sh
	bash test_scripts/test3.sh
