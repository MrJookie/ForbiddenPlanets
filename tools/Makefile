
cpp := map_compiler.cpp

obj := $(patsubst %.cpp, %.o, $(cpp))
		
exec := mapcompile

inc := -I../src -I../includes
link64bit := -L../lib/linux/64bit -lsfml-graphics
link := $(link64bit)

all: $(exec)

clean:
	rm $(obj)

debug := -g

%.o: %.cpp
	g++ -c $< -o $@ -std=c++11 $(inc) $(debug) -O2


$(exec): $(obj)
	g++ $^ -o $@ $(link) $(debug)
