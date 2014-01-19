#include <stdio.h>
#include <stdlib.h>

#include <list.h>
#include <log.h>

#define EE_MALLOC malloc
#define EE_FREE free

typedef void (*ee_cb)(void*);

struct ee__event_s {
  const char *name;
  list_t /*<ee_cb>*/ *handles;
};

typedef struct ee__event_s ee__event_t;

struct ee_s {
  list_t /*<ee__event_t>*/ *events;
};

typedef struct ee_s ee_t;

static int ee__match_events(void* e1, void* e2) {
  ee__event_t *event1;
  ee__event_t *event2;

  event1 = (ee__event_t*)e1;
  event2 = (ee__event_t*)e2;

  return strcmp(event1->name, event2->name) == 0;
}

static void ee__free_event(void* e) {
  ee__event_t* event = (ee__event_t*) e;
  EE_FREE((char*) event->name);
  list_destroy(event->handles);
}

ee_t* ee_new() {
  list_t *events;
  ee_t* self;

  events = list_new();
  events->match = ee__match_events;
  events->free  = ee__free_event;

  self = EE_MALLOC(sizeof *self);
  self->events = events;

  return self;
}

static ee__event_t* ee__find(ee_t* self, const char* name) {
  list_node_t* node;

  ee__event_t proto = { .name = name, .handles = NULL };
  node = (list_node_t*) list_find(self->events, &proto);

  return node != NULL ? (ee__event_t*) node->val : NULL;
}

static ee__event_t* ee__event_new(const char* name) {
  ee__event_t *event;

  event = EE_MALLOC(sizeof *event);
  event->name = strdup(name);
  event->handles = list_new();
  return event;
}

void ee_on(ee_t* self, const char* name, const ee_cb handle) {
  ee__event_t *event;
  list_node_t *handle_node;
  list_node_t *event_node;

  event = ee__find(self, name);
  if (event == NULL) {
    event = ee__event_new(name);
    event_node = list_node_new(event);
    list_rpush(self->events, event_node);
  }

  handle_node = list_node_new(handle);
  list_rpush(event->handles, handle_node);

  log_debug("Added event for '%s'. It has now %d handles", event->name, event->handles->len);
}

void ee_add_listener(ee_t* self, const char* name, const ee_cb handle) {
  ee_on(self, name, handle);
}

void ee_remove_listener(ee_t* self, const char* name, const ee_cb handle) {
  ee__event_t* event;

  event = (ee__event_t*) ee__find(self, name);

  if (event == NULL) {
    log_debug("tried to remove listener for '%s'event that no one listens to", name);
    return;
  }

  list_node_t* handle_node = list_find(event->handles, handle);
  if (handle_node == NULL) {
    log_debug("tried to remove listener for '%s' event but that listener wasn't found", name);
    return;
  }

  log_debug("removing one of %d handles for this event '%s'", event->handles->len, event->name);
  list_remove(event->handles, handle_node);
}

void ee_emit(ee_t* self, const char* name, void* arg) {
  ee__event_t* event;
  list_node_t *node;
  list_iterator_t *it;
  ee_cb handle;

  event = (ee__event_t*) ee__find(self, name);
  if (event == NULL) {
    log_debug("emitted '%s'event that no one listens to", name);
    return;
  }

  it = list_iterator_new(event->handles, LIST_HEAD);

  log_debug("invoking %d handles for this event '%s'", event->handles->len, event->name);

  while((node = list_iterator_next(it))) {
    handle = (ee_cb)node->val;
    handle(arg);
  }
}

void ee_destroy(ee_t* self) {
  list_destroy(self->events);
  EE_FREE(self);
}

void on_error(void* arg) {
  log_debug("on error called: %s", (char*) arg);
}

void on_another_error(void* arg) {
  log_debug("on another error called: %s", (char*) arg);
}

int main(void) {
  ee_t *ee;
  ee = ee_new();
  log_debug("sizes, ee: %ld, cb: %ld", sizeof(*ee), sizeof(ee_cb));

  ee_on(ee, "error", on_error);
  ee_on(ee, "error", on_another_error);

  ee_emit(ee, "error", "1");
  ee_emit(ee, "error", "2");

  ee_remove_listener(ee, "error", on_another_error);
  ee_emit(ee, "error", "3");


  ee_destroy(ee);

  return 0;
}

#undef EE_MALLOC
#undef EE_FREE
