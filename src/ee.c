#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ee.h"

typedef struct {
  const char *name;
  list_t /*<ee_handler_t>*/ *handlers;
} ee__event_t;

static int ee__match_events       ( void* e1, void* e2);
static int ee__match_handlers     ( void* h1, void* h2);
static ee__event_t* ee__find      ( ee_t* self, const char* name);
static ee__event_t* ee__event_new ( const char* name);
static void ee__free_event        ( void* e);

static void ee__on                ( ee_t* self, const char* name, int once, const ee_cb cb);

static int ee__match_events(void* e1, void* e2) {
  ee__event_t *event1;
  ee__event_t *event2;

  event1 = (ee__event_t*)e1;
  event2 = (ee__event_t*)e2;

  return strcmp(event1->name, event2->name) == 0;
}

static int ee__match_handlers(void* h1, void* h2) {
  ee_handler_t *handler1;
  ee_handler_t *handler2;

  handler1 = (ee_handler_t*)h1;
  handler2 = (ee_handler_t*)h2;

  return handler1->cb == handler2->cb;
}

static void ee__free_event(void* e) {
  ee__event_t* event = (ee__event_t*) e;
  free((char*) event->name);
  list_destroy(event->handlers);
}

static ee__event_t* ee__find(ee_t* self, const char* name) {
  list_node_t* node;

  ee__event_t proto = { .name = name, .handlers = NULL };
  node = (list_node_t*) list_find(self->events, &proto);

  return node != NULL ? (ee__event_t*) node->val : NULL;
}

static ee__event_t* ee__event_new(const char* name) {
  ee__event_t *event;

  event                  = malloc(sizeof *event);
  event->name            = strdup(name);
  event->handlers        = list_new();
  event->handlers->match = ee__match_handlers;

  return event;
}

static void ee__on(ee_t* self, const char* name, int once, const ee_cb cb) {
  ee__event_t *event;
  ee_handler_t* handler;
  list_node_t *handler_node;
  list_node_t *event_node;
  int len;

  event = ee__find(self, name);
  if (event == NULL) {
    event = ee__event_new(name);
    event_node = list_node_new(event);
    list_rpush(self->events, event_node);
  }

  handler = malloc(sizeof *handler);
  handler->cb = cb;
  handler->once = once;

  handler_node = list_node_new(handler);
  list_rpush(event->handlers, handler_node);

  if (strcmp(name, EE_NEW_LISTENER)) {
    ee_new_listener_t listener = { .name = name, .cb = cb };
    ee_emit(self, EE_NEW_LISTENER, &listener);
  }

  len = event->handlers->len;
  if (EE_MAX_LISTENERS && len > EE_MAX_LISTENERS) {
    fprintf(stderr, "Event '%s' has %d listeners which exceeds %d max listeners!\n", name, len, EE_MAX_LISTENERS);
    fprintf(stderr, "Increase this max default by setting EE_MAX_LISTENERS - set to zero for unlimited.\n");
  }
}

ee_t* ee_new() {
  list_t *events;
  ee_t* self;

  events = list_new();
  events->match = ee__match_events;
  events->free  = ee__free_event;

  self = malloc(sizeof *self);
  self->events = events;

  return self;
}

void ee_on(ee_t* self, const char* name, const ee_cb handler) {
  ee__on(self, name, 0, handler);
}

void ee_add_listener(ee_t* self, const char* name, const ee_cb handler) {
  ee_on(self, name, handler);
}

void ee_once(ee_t* self, const char* name, const ee_cb handler) {
  ee__on(self, name, 1, handler);
}

void ee_remove_listener(ee_t* self, const char* name, const ee_cb cb) {
  ee__event_t* event;

  event = (ee__event_t*) ee__find(self, name);

  if (event == NULL) return;

  ee_handler_t proto = { .cb = cb };
  list_node_t* handler_node = list_find(event->handlers, &proto);
  if (handler_node == NULL) return;

  list_remove(event->handlers, handler_node);
}

void ee_remove_all_listeners(ee_t* self, const char* name) {
 ee__event_t* event;
  list_node_t *node;
  list_iterator_t *it;

  event = (ee__event_t*) ee__find(self, name);
  it = list_iterator_new(event->handlers, LIST_HEAD);
  while((node = list_iterator_next(it))) {
    list_remove(event->handlers, node);
  }
}

void ee_emit(ee_t* self, const char* name, void* arg) {
  ee__event_t* event;
  list_node_t *node;
  list_iterator_t *it;
  ee_handler_t *handler;

  event = (ee__event_t*) ee__find(self, name);
  if (event == NULL) return;

  it = list_iterator_new(event->handlers, LIST_HEAD);

  while((node = list_iterator_next(it))) {
    handler = (ee_handler_t*)node->val;
    handler->cb(arg);
    if (handler->once) {
      list_remove(event->handlers, node);
    }
  }
}

list_t* ee_listeners(ee_t* self, const char* name) {
  ee__event_t* event;

  event = (ee__event_t*) ee__find(self, name);
  return event != NULL ? event->handlers : NULL;
}

int ee_listener_count(ee_t* self, const char* name) {
  list_t *handlers;
  handlers = ee_listeners(self, name);
  return handlers != NULL ? handlers->len : 0;
}

void ee_destroy(ee_t* self) {
  list_destroy(self->events);
  free(self);
}
