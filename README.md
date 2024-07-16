# Slyce

## Demos

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