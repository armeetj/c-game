// #include "image_wrapper.h"

// #include "aux.h"
// #include "list.h"
// #include <stdbool.h>
// #include <stdlib.h>
// #include "vector.h"
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_image.h>


// typedef struct image_wrapper {
//   char *image_name;
//   char *file_path;
//   double width;
//   double height;
//   vector_t center;
// } image_wrapper_t;

// image_wrapper_t *image_init(char *image_name, char *file_path, double width, double height, vector_t center) {
//   image_wrapper_t *image = malloc(sizeof(image_wrapper_t));
//   image->image_name = image_name;
//   image->file_path = file_path;
//   image->width = width;
//   image->height = height;
//   image->center = center;
//   return image;
// }

// void load_all_images(list_t *all_images, vector_t window_screen) {
//   DL_Window window = SDL_CreateWindow("Image Loading", 0, 0, window_screen.x, window_screen.y, 0);
//   SDL_Renderer renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
//   SDL_RenderClear(renderer);
//   for (size_t i = 0; i < list_size(all_images); i++)
//   {
//     image_wrapper_t curr_image = list_get(all_images, i)
//     SDL_Texture *img = IMG_LoadTexture(renderer, curr_image->file_path);
//     SDL_QueryTexture(img, NULL, NULL, &curr_image->width, &curr_image->height);
//     SDL_Rect texr; texr.x = (center.x - (curr_image->width)/2)/2; texr.y = (center.y + (curr_image->height)/2)/2; texr.w = curr_image->width*2; texr.h = curr_image->height*2; 
//     SDL_RenderCopy(renderer, curr_image, NULL, &texr);
//   }
//   SDL_RenderPresent(renderer);
// }

