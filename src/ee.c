#include <stdio.h>
#include <stdlib.h>
#include <log.h>
#include <list.h>

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

ee_t* ee_new() {
  ee_t* self;
  self = malloc(sizeof *self);
  self->events = list_new();
  return self;
}

static ee__event_t* find(ee_t* self, const char* name) {
  // todo: list_find_cmp
  return (ee__event_t*) list_at(self->events, 0)->val;
}

int ee_on(ee_t* self, const char* name, const ee_cb handle) {
  ee__event_t *event;
  list_node_t *node;

  /* new -- given event for name wasn't found */
  event = (ee__event_t*)malloc(sizeof *event);
  event->name = strdup(name);
  event->handles = list_new();

  node = list_node_new(event);
  list_rpush(self->events, node);

  log_debug("added event for %s", name);
  return 0;
}

int ee_emit(ee_t* self, const char* name, void* arg) {
  ee__event_t* event;
  list_node_t *node;
  list_iterator_t *it;
  ee_cb handle;

  event = find(self, name);
  log_debug("found match %s", event->name);

  it = list_iterator_new(event->handles, LIST_HEAD);

  while((node = list_iterator_next(it))) {
    log_debug("found node invoking handle");
    handle = (ee_cb)node->val;
    handle(arg);
  }
  return 0;
}



void on_error(void* arg) {
  log_debug("on error");
}

int main(void) {
  ee_t *ee;
  ee = ee_new();
  log_debug("sizes, ee: %ld, cb: %ld", sizeof(*ee), sizeof(ee_cb));

  ee_on(ee, "error", on_error);
  ee_emit(ee, "error", "very fatal");

  return 0;
}
