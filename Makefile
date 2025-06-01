SRC=$(wildcard src/*.cpp)
OUT_DIR=build
BIN_NATIVE=$(OUT_DIR)/game
BIN_HTML=$(OUT_DIR)/game.html

CXX=g++
SDL_CFLAGS=$(shell sdl2-config --cflags)
SDL_LDFLAGS=$(shell sdl2-config --libs)
TTF_LDFLAGS=-lSDL2_ttf

all: native wasm-docker

# Native build on host machine (Linux)
native:
	$(CXX) $(SRC) $(SDL_CFLAGS) $(SDL_IMAGE_CFLAGS) $(SDL_LDFLAGS) $(TTF_LDFLAGS) -o $(BIN_NATIVE)

# WebAssembly build using Docker emscripten image
wasm-docker:
	docker run --rm -v $(PWD):/src -w /src -u $(shell id -u):$(shell id -g) emscripten/emsdk \
	em++ $(SRC) -s USE_SDL=2 -s USE_SDL_TTF=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -O2 \
	-o $(BIN_HTML) --preload-file assets --shell-file shell.html

clean:
	rm -rf $(OUT_DIR)/*