all: hotkeyz.debug hotkeyz

hotkeyz.debug:
	clang -g ./Hotkeyz/main.cpp -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -F./Frameworks -lc++ -rpath @executable_path/Frameworks -o $@


hotkeyz:
	clang -g -O3 ./Hotkeyz/main.cpp -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -F./Frameworks -lc++ -rpath @executable_path/Frameworks -o $@

clean:
	git clean -fxd
