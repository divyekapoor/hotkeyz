all: debug release

hotkeyz.debug:
	clang ./Hotkeyz/main.cpp -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -F./Frameworks -lc++ -rpath @executable_path/Frameworks -o $@


hotkeyz.release:
	clang ./Hotkeyz/main.cpp -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -F./Frameworks -lc++ -rpath @executable_path/Frameworks -o $@
