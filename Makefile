MAKEFLAGS += -j$(nproc)

PROJ = tree
BINDIR = bin
ODIR = obj

_DEPS = tree.h
DEPS = $(patsubst %,./%,$(_DEPS))

_OBJ = tree.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

CFLAGS = -I ./include -D _DEBUG -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-check -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,leak,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

SAFETY_COMMAND = set -Eeuf -o pipefail && set -x

$(BINDIR)/$(PROJ): $(ODIR) $(BINDIR) $(OBJ) $(DEPS) lib
	g++ -o $(BINDIR)/$(PROJ) $(OBJ) ./lib/lib.o $(CFLAGS)

run: $(BINDIR)/$(PROJ)
	$(BINDIR)/$(PROJ) in.txt out.txt

clean:
	$(SAFETY_COMMAND) && rm -rf $(ODIR) $(BINDIR)

test: $(BINDIR)
	g++ -o $(BINDIR)/$(PROJ)_test file.cpp main.cpp sort.cpp test.cpp hashmap.cpp bits.cpp prefixes.cpp $(CFLAGS) -D TEST && $(BINDIR)/$(PROJ)_test

.PHONY: clean lib

lib:
	cd lib && g++ $(CFLAGS) -c -o lib.o log.cpp

$(ODIR):
	mkdir -p $(ODIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(ODIR)/%.o: %.cpp $(DEPS)
	g++ -c -o $@ $< $(CFLAGS)
