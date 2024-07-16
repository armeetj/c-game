# Slyce

A fully functional multiplayer game and low level game/physicsl engine written in C and SDL2 and compiled to the browser with Emscripten.

This repository contains...
1. the core game/physics engine in `/game/library`
2. the game, SLYCE in `/game`
3. physics demos in `/demos`

## Table of Contents
- [Slyce](#slyce)
  - [Table of Contents](#table-of-contents)
  - [Game Demo](#game-demo)
  - [Game/Physics Demos](#gamephysics-demos)
  - [File Structure](#file-structure)
  - [Documentation](#documentation)
  - [Build](#build)
    - [Prerequisites](#prerequisites)
    - [Steps](#steps)

## Game Demo

https://github.com/user-attachments/assets/851a541b-4b74-4bd2-af0e-5b727c7a369e

Also available at
https://www.youtube.com/watch?v=8B1DQi7yPfE

## Game/Physics Demos

https://github.com/user-attachments/assets/aa94d6c7-b234-4364-ba45-c35ad94f59cd

https://github.com/user-attachments/assets/334fcb4f-7d08-4e13-8266-72c8fd0815aa

https://github.com/user-attachments/assets/63fa7255-9c5b-444b-9d9d-a766f0370e24

https://github.com/user-attachments/assets/099a373e-39a0-423f-a986-c527e0cd15c1

https://github.com/user-attachments/assets/349af66a-2a09-49db-b11b-013e9387c122

https://github.com/user-attachments/assets/d80159d4-cc54-4c8d-82b8-24817647b377

https://github.com/user-attachments/assets/8b2a10d4-c658-43a3-9ed6-d3af126c627a

## File Structure
- **game**: Contains the final product, the game, SLYCE.
  - **game/assets**: Game assets, audio, images, fonts, etc.
  - **game/bin**: Final static targets.
  - **game/demo/slyce.c**: Main entry, game loop.
  - **game/include**: Library headers.
  - **game/library**: Library implementations.
  - **game/out**: Build artifacts.
- **demos:** Selected video demonstrations.
- **archive**: Contains archives of physics engine demos.
## Documentation

## Build
### Prerequisites
1. Install SDL2 for your system.
```bash
sudo apt-get install libsdl2-dev
```

2. Next install SDL2_mixer from https://github.com/libsdl-org/SDL_mixer/releases

   On a Debian Linux based machine, this requires cloning the repo, running `bash configure.sh` and then `make install`

Install emscripten for your system.
```bash
git clone https://github.com/emscripten-core/emsdk.git
```
```bash
cd emsdk
```
```bash
./emsdk install latest
```
```bash
./emsdk activate latest
```
```bash
source ./emsdk_env.sh
```
### Steps
1. Clone this repository
2.
    ```bash
    git clone <link>
    ```
1. Change directory
   ```bash
   cd game-engine/slyce
    ```
1. Run build command
   ```bash
   make
    ```
5. Open game in browser. Choose fullscreen mode (show cursor).
   ```bash
   http://localhost:8000/bin/slyce.html
    ```
