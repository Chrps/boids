# Boids

A simple boids simulation written in C++ using SDL2 for rendering.   
This project can be built for native Linux or as a WebAssembly app to run in the browser.

---

## 🔗 Live Demo

See the simulation in action on GitHub Pages:  
[https://chrps.github.io/boids/](https://chrps.github.io/boids/)

---

## 🛠 Installation

Make sure you have the SDL2 development libraries installed on your system:

```bash
sudo apt-get install libsdl2-dev libsdl2-ttf-dev
```

To build WebAssembly version docker is also required.

## ⚙️ Build Instructions

This project uses a Makefile with targets for native and WebAssembly builds.

### Build all targets (native + wasm):   
```bash
make all
```

### Build native executable (Linux):
```bash
make native
```

### Build WebAssembly version (using Docker and Emscripten):
```bash
make wasm-docker
```

# 📁 Preparing for GitHub Pages

Before pushing your WebAssembly build to GitHub Pages, run the following script to prepare the docs folder:
```bash
./prepare_docs.sh
```
This script will:
   * Delete any existing docs folder
   * Copy the build folder to docs
   * Rename game.html to index.html
   * Remove the native binary game from docs

The docs folder is the source for the GitHub Pages deployment (for this repo).

# 📝 TODO

These are the things I would like to do in the future
   * Toggle settings visibility
   * More settings in the UI
   * Make the boids prettier
      * Other shape (triangle maybe?)
      * Trails
      * Vectors (showing the different forces)
   * Optimize the program for the CPU
   * Move to GPU rendering using WebGL + GLSL or WebGPU