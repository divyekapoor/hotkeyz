all: debug release

debug:
	clang -v -g -I /usr/local/include/ -L /usr/local/lib/ -L /usr/lib main.cpp -lSDLmain -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf -lc++ -framework Cocoa -o debug

release:
	clang -v -g -I /usr/local/include/ -L /usr/local/lib/ -L /usr/lib main.cpp -lSDLmain -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf -lc++ -framework Cocoa -O3 -o release

