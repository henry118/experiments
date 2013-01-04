CC = g++
OBJS = main.o

main: $(OBJS)

.PHONY: clean
clean:
	rm -f *.o my
