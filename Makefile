CC     = clang++
CFLAGS = -std=c++20 -Weverything -Wno-missing-prototypes -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-weak-vtables -Wno-padded -Wno-exit-time-destructors
DEPS   = token.hpp lex.hpp parse.hpp exceptions.hpp astnode.hpp
OBJ    = main.o
LIBS   = -lm

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

compiler: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -rf *.o compiler