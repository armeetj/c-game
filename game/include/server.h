#ifndef __SERVER_H__
#define __SERVER_H__

#include "./list.h";

#include <stdint.h>

typedef struct server
{
  list_t *clients;
  uint16_t socket_id;
  uint16_t ip;
  uint16_t port;
} server_t;

server_t *server_init(uint16_t ip, uint16_t port);

void server_bind(server_t *s);

void server_listen(server_t *s);

void *server_close(server_t *s);

void server_free(server_t *s);

#endif