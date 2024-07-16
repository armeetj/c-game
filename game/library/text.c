#include "sdl_wrapper.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "text.h"

text_t *text_init(char *text, vector_t center, double height, double width, color_t color, double duration) {
  text_t *t = malloc(sizeof(text_t));
  t->text = text;
  t->center = center;
  t->height = height;
  t->width = width;
  t->color = color;
  t->duration = duration;
  t->texture = sdl_prepare_text(text, center, height, width * strlen(text), color);
  t->removed = false;
  return t;
}

void text_edit(text_t *t, char *new) {
  // new text contents
  free(t->text);
  t->text = malloc(sizeof(char) * strlen(new));
  strcpy(t->text, new);

  // new texture
  sdl_free_text(t->texture);
  t->texture = sdl_prepare_text(new, t->center, t->height, t->width * strlen(new), t->color);
}

void text_set_color(text_t *t, color_t new_color) {
  sdl_free_text(t->texture);
  t->color = new_color;
  t->texture = sdl_prepare_text(t->text, t->center, t->height, t->width * strlen(t->text), new_color);
}

void text_move(text_t *t, vector_t pos) {
  t->center = pos;
  sdl_move_text(t->texture, pos, t->height, t->width * strlen(t->text));
}

void text_render(text_t *t) {
  sdl_render_text(t->texture);
}

void text_remove(text_t *t) {
  if(!t->removed) {
    t->removed = true;
    t->duration = 0;
  }
}

void text_tick(text_t *t, double dt) {
  if(t->duration != INFINITY) {
    t->duration -= dt;
    if(t->duration <= 0) {
      t->duration = 0;
      text_remove(t);
    }
  }
  
  if(!t->removed) {
    text_render(t);
  } 
}

void text_free(void *t) {
  text_t *t_casted = (text_t *)t;
  sdl_free_text(t_casted->texture);
  free(t_casted->text);
  free(t_casted);
}