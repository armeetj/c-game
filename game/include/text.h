#ifndef __TEXT_H__
#define __TEXT_H__

typedef struct text {
  char *text;
  vector_t center;
  double height;
  double width;
  color_t color;
  double duration;
  list_t *texture;
  bool removed;
} text_t;

text_t *text_init(char *text, vector_t center, double height, double width, color_t color, double duration);

void text_edit(text_t *t, char *new);

void text_set_color(text_t *t, color_t new_color);

void text_move(text_t *t, vector_t pos);

void text_render(text_t *t);

void text_remove(text_t *t);

void text_tick(text_t *t, double dt);

void text_free(void *t);

#endif