CC = g++
OBJS = main.o

main: $(OBJS)

.PHONY: clean
clean:
	$(RM) *.o main
