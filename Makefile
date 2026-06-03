CC     = clang++-22
CFLAGS = -std=c++23 -stdlib=libc++ -Wall -Wextra -Werror
DEPS   = token.hpp lex.hpp parse.hpp exceptions.hpp astnode.hpp bimap.hpp
OBJ    = main.o
LIBS   = -lm

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

compiler: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -rf *.o compiler