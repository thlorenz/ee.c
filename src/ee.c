#include <stdio.h>
#include <stdlib.h>

#include <list.h>
#include <log.h>

#define EE_NEW_LISTENER "new_listener"

typedef void (*ee_cb)(void*);
typedef void (*ee_new_listener_cb)(const char*);

typedef struct {
  const char *name;
  list_t /*<ee_cb>*/ *handlers;
} ee__event_t;

typedef struct {
  const char *name;
  ee_cb handler;
} ee_listener_t;

typedef struct {
  list_t /*<ee__event_t>*/ *events;
} ee_t;

void ee_on              ( ee_t* self, const char* name, const ee_cb handler);
void ee_add_listener    ( ee_t* self, const char* name, const ee_cb handler);
void ee_once            ( ee_t* self, const char* name, const ee_cb handler);
void ee_remove_listener ( ee_t* self, const char* name, const ee_cb handler);
void ee_emit            ( ee_t* self, const char* name, void* arg);

list_t* ee_listeners    ( ee_t* self, const char* name);
int ee_listener_count   ( ee_t* self, const char* name);


static int ee__match_events(void* e1, void* e2) {
  ee__event_t *event1;
  ee__event_t *event2;

  event1 = (ee__event_t*)e1;
  event2 = (ee__event_t*)e2;

  return strcmp(event1->name, event2->name) == 0;
}

static void ee__free_event(void* e) {
  ee__event_t* event = (ee__event_t*) e;
  free((char*) event->name);
  list_destroy(event->handlers);
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

static ee__event_t* ee__find(ee_t* self, const char* name) {
  list_node_t* node;

  ee__event_t proto = { .name = name, .handlers = NULL };
  node = (list_node_t*) list_find(self->events, &proto);

  return node != NULL ? (ee__event_t*) node->val : NULL;
}

static ee__event_t* ee__event_new(const char* name) {
  ee__event_t *event;

  event = malloc(sizeof *event);
  event->name = strdup(name);
  event->handlers = list_new();
  return event;
}

void ee_on(ee_t* self, const char* name, const ee_cb handler) {
  ee__event_t *event;
  list_node_t *handler_node;
  list_node_t *event_node;

  event = ee__find(self, name);
  if (event == NULL) {
    event = ee__event_new(name);
    event_node = list_node_new(event);
    list_rpush(self->events, event_node);
  }

  handler_node = list_node_new(handler);
  list_rpush(event->handlers, handler_node);

  if (strcmp(name, EE_NEW_LISTENER)) {
    ee_listener_t listener = { .name = name, .handler = handler };
    ee_emit(self, EE_NEW_LISTENER, &listener);
  }

  log_debug("Added event for '%s'. It has now %d handlers", event->name, event->handlers->len);
}

void ee_add_listener(ee_t* self, const char* name, const ee_cb handler) {
  ee_on(self, name, handler);
}

void ee_remove_listener(ee_t* self, const char* name, const ee_cb handler) {
  ee__event_t* event;

  event = (ee__event_t*) ee__find(self, name);

  if (event == NULL) {
    log_debug("tried to remove listener for '%s'event that no one listens to", name);
    return;
  }

  list_node_t* handler_node = list_find(event->handlers, handler);
  if (handler_node == NULL) {
    log_debug("tried to remove listener for '%s' event but that listener wasn't found", name);
    return;
  }

  log_debug("removing one of %d handlers for this event '%s'", event->handlers->len, event->name);
  list_remove(event->handlers, handler_node);
}

void ee_once(ee_t* self, const char* name, const ee_cb handler) {
  // TODO
}

void ee_emit(ee_t* self, const char* name, void* arg) {
  ee__event_t* event;
  list_node_t *node;
  list_iterator_t *it;
  ee_cb handler;

  event = (ee__event_t*) ee__find(self, name);
  if (event == NULL) {
    log_debug("emitted '%s'event that no one listens to", name);
    return;
  }

  it = list_iterator_new(event->handlers, LIST_HEAD);

  log_debug("invoking %d handlers for this event '%s'", event->handlers->len, event->name);

  while((node = list_iterator_next(it))) {
    handler = (ee_cb)node->val;
    handler(arg);
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

void on_error(void* arg) {
  log_debug("on error called: %s", (char*) arg);
}

void on_another_error(void* arg) {
  log_debug("on another error called: %s", (char*) arg);
}

void on_new_listener(void* arg) {
  ee_listener_t *listener;
  listener = (ee_listener_t*) arg;
  log_debug("Added listener for '%s'", listener->name);
}

int main(void) {
  ee_t *ee;
  ee = ee_new();
  log_debug("sizes, ee: %ld, cb: %ld", sizeof(*ee), sizeof(ee_cb));

  ee_on(ee, "new_listener", on_new_listener);

  ee_on(ee, "error", on_error);
  ee_on(ee, "error", on_another_error);
  log_debug("error has %d listeners", ee_listener_count(ee, "error"));

  ee_emit(ee, "error", "1");
  ee_emit(ee, "error", "2");

  ee_remove_listener(ee, "error", on_another_error);
  log_debug("error has %d listeners", ee_listener_count(ee, "error"));
  ee_emit(ee, "error", "3");


  ee_destroy(ee);

  return 0;
}
