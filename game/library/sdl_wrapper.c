#include "sdl_wrapper.h"
#include "list.h"
#include "body.h"
#include "scene.h"
#include "state.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>

const char WINDOW_TITLE[] = "SLYCE";
const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
const double MS_PER_S = 1e3;

const TTF_Font *font;
const double frequency = 44100;
const int channels = 2;
const int chunk_size = 2048;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

typedef struct context {
    SDL_Rect dest;
    SDL_Texture *ashug_tex;

    TTF_Font *font;
    SDL_Texture *text_tex;

    vector_t ashug_vec;
} context_t;


vector_t get_mouse_pos(void) {
  int *x = malloc(sizeof(int));
  int *y = malloc(sizeof(int));
  SDL_GetMouseState(x, y);
  vector_t dimensions = {.x = *x, .y = *y};
  free(x);
  free(y);
  return dimensions;
}

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  font = TTF_OpenFont("assets/joystix.ttf", 100);

  // init mixer
  if( Mix_OpenAudio( frequency, MIX_DEFAULT_FORMAT, channels, chunk_size ) < 0 ) {
      printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
  }

  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
}

bool sdl_is_done(state_t *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(state, key, type, held_time);
      break;
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, color_t color) {
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color.r && color.r <= 1);
  assert(0 <= color.g && color.g <= 1);
  assert(0 <= color.b && color.b <= 1);
  assert(0 <= color.a && color.a <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, color.a * 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene) {
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    sdl_draw_polygon(shape, body_get_color(body));
    list_free(shape);
  }
  sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

void sdl_move_text(list_t *l, vector_t pos, double height, double width) {
  SDL_Rect *message_rectangle = (SDL_Rect *) list_get(l, 1);
  message_rectangle->x = pos.x - width / 2;
  message_rectangle->y = WINDOW_HEIGHT - pos.y - height / 2;
  message_rectangle->w = width;
  message_rectangle->h = height;
}

list_t *sdl_prepare_text(char* text, vector_t center, double height, double width, color_t color) {
  // texture prep
  SDL_Color text_color = {color.r*255, color.g*255, color.b*255, color.a*255};
  SDL_Surface* message_surface = TTF_RenderText_Solid(font, text, text_color);
  SDL_Texture* message_texture = SDL_CreateTextureFromSurface(renderer, message_surface);

  // rectangle prep (placement)
  SDL_Rect *message_rectangle = malloc(sizeof(SDL_FRect));
  message_rectangle->x = center.x - width / 2;
  message_rectangle->y = WINDOW_HEIGHT - center.y - height / 2;
  message_rectangle->w = width;
  message_rectangle->h = height;

  list_t *l = list_init(2, NULL);
  list_add(l, message_texture);
  list_add(l, message_rectangle);
  list_add(l, message_surface);
  return l;
}

void sdl_render_text(list_t *l) {
  SDL_Texture *message_texture = list_get(l, 0);
  SDL_Rect *message_rectangle = list_get(l, 1);
  SDL_RenderCopy(renderer, message_texture, NULL, message_rectangle);
  sdl_show();
}

void sdl_free_text(list_t *l) {
  SDL_Texture *message_texture = (SDL_Texture *) list_get(l, 0);
  SDL_Rect *message_rectangle = (SDL_Rect *) list_get(l, 1);
  SDL_Surface *message_surface = (SDL_Surface *) list_get(l, 2);
  SDL_FreeSurface(message_surface);
  SDL_DestroyTexture(message_texture);
  free(message_rectangle);
}

// disabled for compatibility
void sdl_play_music(char *path) {
//   Mix_Music *sound = Mix_LoadMUS(path);
//   Mix_PlayMusic(sound, 0);
}
// disabled for compatibility
void sdl_play_sound(int channel, char *path, int loops) {
//   Mix_Chunk *chunk = Mix_LoadWAV(path);
//   Mix_PlayChannel(channel, chunk, loops);
}

void sdl_render_image() {
  SDL_Surface * image = SDL_LoadBMP("assets/background.bmp");
  SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, image);

  SDL_RenderCopy(renderer, texture, NULL, NULL);

  SDL_RenderPresent(renderer);
  sdl_show();
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(image);
}
