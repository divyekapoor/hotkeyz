all: hotkeyz.debug hotkeyz.release

hotkeyz.debug: ./Hotkeyz/main.cpp
	clang -g ./Hotkeyz/main.cpp -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -F./Frameworks -lc++ -rpath @executable_path/Frameworks -o $@


hotkeyz.release: ./Hotkeyz/main.cpp
	clang -g -O3 ./Hotkeyz/main.cpp -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -F./Frameworks -lc++ -rpath @executable_path/Frameworks -o $@

clean:
	git clean -fxd
