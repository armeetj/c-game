# Slyce Game/Physics Engine

A fully functional multiplayer game and low level game/physicsl engine written in C and SDL2 and compiled to the browser with Emscripten.

This repository contains...
1. the core game/physics engine in `/game/library` and documentation see _Table of Contents_
2. the game, SLYCE in `/game`
3. physics demos in `/demos`

## Game Demo

https://github.com/user-attachments/assets/851a541b-4b74-4bd2-af0e-5b727c7a369e

Also available at
https://www.youtube.com/watch?v=8B1DQi7yPfE

## Table of Contents
- [Slyce](#slyce)
  - [Game Demo](#game-demo)
  - [Table of Contents](#table-of-contents)
  - [Game/Physics Demos](#gamephysics-demos)
  - [File Structure](#file-structure)
  - [Documentation](#documentation)
  - [Build](#build)
    - [Prerequisites](#prerequisites)
    - [Steps](#steps)

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
The library is directly documented in the source code. The following summaries are LLM generated and give an overview of each module of the full library.

#### `aux.c`

This module handles auxiliary data structures and operations. It manages lists of constants and bodies used throughout the simulation.

##### Includes
```c
#include "aux.h"
#include "list.h"
```

##### Structs
**`aux_t`**: This structure holds lists of constants and bodies.

##### Functions
- `aux_t *aux_init(list_t *constants, list_t *bodies)`
- `list_t *aux_get_bodies(aux_t *aux)`
- `list_t *aux_get_constants(aux_t *aux)`
- `void aux_free(void *aux)`


#### `body.c`

This module defines the body structure and related functions. It represents physical bodies in the simulation, including their shapes, masses, and colors.

##### Includes
```c
#include "body.h"
#include "list.h"
```

##### Structs
**`body_t`**: Represents a physical body in the simulation.

##### Functions
- `body_t *body_init(list_t *shape, double mass, rgb_color_t color)`
- `void body_free(void *body)`
- `list_t *body_get_shape(body_t *body)`
- `double body_get_mass(body_t *body)`


#### `collision.c`

This module contains functions related to collision detection between shapes. It helps in determining if and where collisions occur within the simulation.

##### Includes
```c
#include "collision.h"
```

##### Functions
- `bool find_collision(list_t *shape1, list_t *shape2)`
- `bool find_collision_aux(list_t *shape1, list_t *shape2)`


#### `collision_package.c`

This module manages collision packages which encapsulate collision information between two bodies. It provides structures and functions to handle collision data efficiently.

##### Includes
```c
#include "collision_package.h"
```

##### Structs
**`collision_package_t`**: Encapsulates collision information between two bodies.

##### Functions
- `collision_package_t *collision_package_init(body_t *body1, body_t *body2)`
- `void collision_package_free(void *collision_package)`
- `body_t *collision_package_get_body1(collision_package_t *collision_package)`
- `body_t *collision_package_get_body2(collision_package_t *collision_package)`


#### `color.c`

This module handles color data, specifically dealing with the `rgb_color_t` structure which represents a color with red, green, and blue components.

##### Includes
```c
#include "color.h"
```

##### Structs
**`rgb_color_t`**: Represents a color with red, green, and blue components.

##### Functions
- `rgb_color_t color_init(double r, double g, double b)`


#### `emscripten.c`

This module provides functions to initialize and manage the Emscripten environment, facilitating the interaction between C code and JavaScript.

##### Includes
```c
#include "emscripten.h"
```

##### Functions
- `void emscripten_init(void)`
- `void emscripten_free(void)`


#### `forces.c`

This module defines forces and their application on bodies within the simulation. It includes structures and functions to represent and manipulate physical forces.

##### Includes
```c
#include "forces.h"
```

##### Structs
**`force_t`**: Represents a physical force.

##### Functions
- `force_t *force_init(double magnitude, double direction)`
- `void force_free(void *force)`
- `void force_apply(force_t *force, body_t *body)`


#### `force_wrapper.c`

This module manages force wrappers which encapsulate forces for easy manipulation and application.

##### Includes
```c
#include "force_wrapper.h"
```

##### Structs
**`force_wrapper_t`**: Wraps a force for easier management.

##### Functions
- `force_wrapper_t *force_wrapper_init(force_t *force)`
- `void force_wrapper_free(void *force_wrapper)`


#### `image_wrapper.c`

This module handles image data, providing structures and functions to manage images within the simulation.

##### Includes
```c
#include "image_wrapper.h"
```

##### Structs
**`image_wrapper_t`**: Wraps an image for easier management.

##### Functions
- `image_wrapper_t *image_wrapper_init(const char *filename)`
- `void image_wrapper_free(void *image_wrapper)`


#### `list.c`

This module defines and manages dynamic arrays, providing structures and functions to handle lists of data.

##### Includes
```c
#include "list.h"
```

##### Structs
**`list_t`**: Represents a dynamic array.

##### Functions
- `list_t *list_init(size_t initial_size)`
- `void list_free(list_t *list)`
- `void list_add(list_t *list, void *value)`
- `void *list_get(list_t *list, size_t index)`
- `void list_remove(list_t *list, size_t index)`


#### `player.c`

This module manages player data within the game, including player initialization, data retrieval, and modification.

##### Includes
```c
#include "player.h"
```

##### Structs
**`player_t`**: Represents a player in the game.

##### Functions
- `player_t *player_init(const char *name, int score)`
- `void player_free(void *player)`
- `const char *player_get_name(player_t *player)`
- `int player_get_score(player_t *player)`
- `void player_set_score(player_t *player, int score)`


#### `polygon.c`

This module provides functions for polygon operations, including initialization, area calculation, and vertex management.

##### Includes
```c
#include "polygon.h"
```

##### Functions
- `polygon_t *polygon_init(list_t *vertices)`
- `void polygon_free(polygon_t *polygon)`
- `list_t *polygon_get_vertices(polygon_t *polygon)`
- `double polygon_area(polygon_t *polygon)`


#### `scene.c`

This module manages scenes containing multiple bodies. It provides structures and functions to handle scene initialization, body management, and scene updates.

##### Includes
```c
#include "scene.h"
```

##### Structs
**`scene_t`**: Represents a scene containing multiple bodies.

##### Functions
- `scene_t *scene_init(void)`
- `void scene_free(scene_t *scene)`
- `void scene_add_body(scene_t *scene, body_t *body)`
- `body_t *scene_get_body(scene_t *scene, size_t index)`


#### `sdl_wrapper.c`

This module contains functions for initializing and managing the SDL environment, including drawing operations and screen updates.

##### Includes
```c
#include "sdl_wrapper.h"
```

##### Functions
- `void sdl_init(void)`
- `void sdl_clear(void)`
- `void sdl_draw_polygon(list_t *points, rgb_color_t color)`
- `void sdl_show(void)`


#### `server.c`

This module contains functions related to server operations, including initialization, listening, sending, and receiving data.

##### Includes
```c
#include "server.h"
```

##### Functions
- `void server_init(void)`
- `void server_listen(void)`
- `void server_send(const char *message)`
- `char *server_receive(void)`


#### `state.c`

This module manages the state of the game or simulation, including state initialization, updates, and rendering.

##### Includes
```c
#include "state.h"
```

##### Structs
**`state_t`**: Represents the state of the game or simulation.

##### Functions
- `state_t *state_init(void)`
- `void state_free(state_t *state)`
- `void state_update(state_t *state)`
- `void state_render(state_t *state)`


#### `test_util.c`

This module contains utility functions for testing, including assertion checks and running test suites.

##### Includes
```c
#include "test_util.h"
```

##### Functions
- `bool test_util_approx_eq(double a, double b)`
- `void test_util_assert(bool condition)`
- `void test_util_run_tests(void)`


#### `text.c`

This module manages text rendering within the simulation, including text initialization, drawing, and freeing resources.

##### Includes
```c
#include "text.h"
```

##### Functions
- `void text_init(void)`
- `void text_draw(const char *text, double x, double y, rgb_color_t color)`
- `void text_free(void)`


#### `utils.c`

This module contains various utility functions, including random number generation, clamping values, and sleeping.

##### Includes
```c
#include "utils.h"
```

##### Functions
- `double utils_rand_range(double min, double max)`
- `int utils_clamp(int value, int min, int max)`
- `void utils_sleep(int milliseconds)`


#### `vector.c`

This module deals with 2D vector operations, providing structures and functions for vector arithmetic and manipulation.

##### Includes
```c
#include "vector.h"
```

##### Structs
**`vector_t`**: Represents a 2D vector.

##### Functions
- `vector_t vector_add(vector_t v1, vector_t v2)`
- `vector_t vector_subtract(vector_t v1, vector_t v2)`
- `vector_t vector_scale(vector_t v, double scalar)`
- `double vector_dot(vector_t v1, vector_t v2)`
- `double vector_cross(vector_t v1, vector_t v2)`
- `double vector_magnitude(vector_t v)`
- `vector_t vector_normalize(vector_t v)`

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
3. Change directory
   ```bash
   cd game-engine/slyce
    ```
4. Run build command
   ```bash
   make NO_ASAN=true all
    ```
  ASAN is a debugging tool and not needed in the final product. <br/>
  If you forget to include the NO_ASAN flag, performance will suffer and demos will stutter.
5. Open game in browser. Choose fullscreen mode (show cursor).
   ```bash
   http://localhost:8000/bin/slyce.html
    ```
