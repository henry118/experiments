OBJS = main.o

my: $(OBJS)
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.PHONY: clean
clean:
	rm *.o my
