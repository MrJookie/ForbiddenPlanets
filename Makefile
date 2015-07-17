inc := -Iinclude -Isrc/Micropather
link64bit := -Llib/linux/64bit -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-system \
				-lsfml-network -Wl,-rpath="../lib/linux/64bit/",-rpath="../lib/linux"  -Llib/linux -lsfgui
link32bit := -Llib/linux/32bit -lsfml-window  -lsfml-audio -lsfml-graphics -lsfml-system \
				-lsfml-network -Wl,-rpath="../lib/linux/32bit/",-rpath="../lib/linux" -Llib/linux -lsfgui
link := $(link64bit)

arch := 

cpp := 	\
		src/Application.cpp				\
		src/GameState.cpp               \
		src/MenuState.cpp               \
		src/Player.cpp                  \
		src/ResourceManager.cpp         \
		src/TileMap.cpp                 \
		src/Micropather/Micropather.cpp	\
		src/Main.cpp					\

exe := Release/ForbiddenPlanets

build := build

obj := $(addprefix $(build)/, $(patsubst %.cpp,%.o,$(cpp)))

.phony: make_dirs

all: make_dirs $(exe)

clean:
	find $(build) -type f -name *.o -exec rm {} \;

make_dirs:
	@mkdir -p $(build)
	@mkdir -p $(build)/src
	@mkdir -p $(build)/src/Micropather

$(exe): $(obj)
	g++ $^ -o $(exe) $(link) $(arch)

$(build)/%.o: %.cpp
	g++ -c $< -o $@ -std=c++11 $(inc) $(arch) -O2
