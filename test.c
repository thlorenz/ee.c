#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "src/ee.h"

#define MAX_NEW_LISTENERS

#define test(fn) \
  fputs("\n\x1b[34m# " # fn "\x1b[0m\n", stderr); \
  fn();

#define t_ok(pred, msg) do {                       \
  fprintf(stderr, "  - \x1b[90m" msg "\x1b[0m\n"); \
  assert(pred);                                    \
} while(0)

#define t_equal_str(s1, s2, msg) t_ok(strcmp((s1), (s2)) == 0, msg)

typedef struct {
  int count;
  ee_new_listener_t listeners[MAX_NEW_LISTENERS];
} ee_new_listeners_t;

static ee_new_listeners_t new_listeners;

void on_added_new_listener(void* arg) {
  ee_new_listener_t listener;
  ee_new_listener_t *larg;
  larg = (ee_new_listener_t*) arg;

  /* make a copy of the passed listener since that one is automatic and will be gone by the time we leave scope */
  listener = new_listeners.listeners[new_listeners.count];
  listener.name = strdup(larg->name);
  listener.cb = larg->cb;

  new_listeners.count++;
}

static char* on_hello_arg;
static int on_hello_count;
static void on_hello(void* arg) {
  on_hello_arg = arg;
  on_hello_count++;
}

static char* on_hello_again_arg;
static int on_hello_again_count;
static void on_hello_again(void* arg) {
  on_hello_again_arg = arg;
  on_hello_again_count++;
}

static char* on_hi_arg;
static int on_hi_count;
static void on_hi(void* arg) {
  on_hi_arg = arg;
  on_hi_count++;
}

static char* on_hi_again_arg;
static int on_hi_again_count;
static void on_hi_again(void* arg) {
  on_hi_again_arg = arg;
  on_hi_again_count++;
}

static void reset_counts() {
  on_hello_count = 0;
  on_hello_again_count = 0;
  on_hi_count = 0;
  on_hi_again_count = 0;
}

static void setup() {
  int i;
  ee_new_listener_t listener;

  for (i = 0; i < new_listeners.count; i++) {
    listener = new_listeners.listeners[i];
    free((char*)listener.name);
  }
  new_listeners.count = 0;

  reset_counts();

  on_hello_arg = NULL;
  on_hello_again_arg = NULL;
  on_hi_arg = NULL;
  on_hi_again_arg = NULL;
}

void add_listener() {
  setup();
  ee_t *ee = ee_new();
  ee_on(ee, EE_NEW_LISTENER, on_added_new_listener);

  t_ok(new_listeners.count == 0, "new listener never triggers initially");

  ee_add_listener(ee, "hello", on_hello);
  t_ok(new_listeners.count == 1, "new listener triggers once after adding one new event");

  ee_on(ee, "hello", on_hello_again);
  t_ok(new_listeners.count == 2, "new listener triggers again after adding another new event");

  t_ok(ee_listener_count(ee, "hello") == 2, "the two listeners are counted correctly for that event");
  t_ok(ee_listener_count(ee, "other") == 0, "listener count for some other event is 0");

  list_t *listeners;
  listeners = ee_listeners(ee, "hello");
  ee_handler_t *head_handler = (ee_handler_t*)listeners->head->val;
  ee_handler_t *next_handler = (ee_handler_t*)listeners->head->next->val;
  ee_handler_t *tail_handler = (ee_handler_t*)listeners->tail->val;
  t_ok(head_handler->cb == on_hello, "first added listener is first in list of listeners");
  t_ok(next_handler->cb == on_hello_again, "next added listener is next in list of listeners");
  t_ok(tail_handler->cb == on_hello_again, "last added listener is last in list of listeners");

  listeners = ee_listeners(ee, "hi");
  t_ok(listeners == NULL, "when asking for listeners of event without listeners we get NULL");

  ee_destroy(ee);
}

void emit() {
  setup();
  ee_t *ee = ee_new();

  ee_emit(ee, "hello", "world");
  t_ok(on_hello_arg == NULL, "emitting 'hello' before adding listener goes unnoticed");
  assert(on_hello_count == 0);
  t_ok(on_hello_again_arg == NULL, "for both callbacks");
  assert(on_hello_again_count == 0);

  ee_on(ee, "hello", on_hello);

  ee_emit(ee, "hello", "world");
  t_equal_str(on_hello_arg, "world", "emitting 'hello' after adding one listener triggers added callback");
  assert(on_hello_count == 1);
  t_ok(on_hello_again_arg == NULL, "but not the one that wasn't added yet");
  assert(on_hello_again_count == 0);

  on_hello_arg = NULL;
  ee_on(ee, "hello", on_hello_again);

  ee_emit(ee, "hello", "world");
  t_equal_str(on_hello_arg, "world", "emitting 'hello' after adding another listener triggers first added callback");
  assert(on_hello_count == 2);
  t_equal_str(on_hello_again_arg, "world", "and the last added callback");
  assert(on_hello_again_count == 1);

  ee_destroy(ee);
}

void once() {
  setup();
  ee_t *ee = ee_new();

  ee_once(ee, "hello", on_hello);

  ee_emit(ee, "hello", "world");
  t_equal_str(on_hello_arg, "world", "emitting 'hello' subscribing to it once, calls the handler");
  assert(on_hello_count == 1);

  on_hello_arg = NULL;
  ee_emit(ee, "hello", "world");
  t_ok(on_hello_arg == NULL, "emitting 'hello' again doesn't call the handler a second time");
  assert(on_hello_count == 1);

  ee_destroy(ee);
}


void remove_listener() {
  setup();
  ee_t *ee = ee_new();

  ee_on(ee, "hello", on_hello);
  ee_on(ee, "hello", on_hello_again);
  ee_on(ee, "hello", on_hello);
  ee_on(ee, "hello", on_hello);
  t_ok(ee_listener_count(ee, "hello") == 4, "given 4 listeners for 'hello' event");

  ee_on(ee, "hi", on_hi);
  ee_on(ee, "hi", on_hi_again);
  t_ok(ee_listener_count(ee, "hi") == 2, "and 2 listeners for 'hi' event");

  ee_emit(ee, "hello", "world");
  t_ok(on_hello_count == 3, "when emitting hello it is called for each registered handler");
  assert(on_hello_again_count == 1);

  ee_emit(ee, "hi", "world");
  t_ok(on_hi_count == 1, "when emitting hi it is called for each registered handler");
  assert(on_hi_again_count == 1);

  reset_counts();
  ee_remove_listener(ee, "hello", on_hello);

  ee_emit(ee, "hello", "world");
  t_ok(on_hello_count == 2, "when emitting hello after removing one handler it is called for each still registered handler");
  assert(on_hello_again_count == 1);

  ee_emit(ee, "hi", "world");
  t_ok(on_hi_count == 1, "when emitting hi it is called for each registered handler");
  assert(on_hi_again_count == 1);

  reset_counts();
  ee_remove_listener(ee, "hi", on_hi_again);

  ee_emit(ee, "hi", "world");
  t_ok(on_hi_count == 1, "when emitting hi after removing on_hi_again handler it only called of on_hi handler");
  assert(on_hi_again_count == 0);

  ee_destroy(ee);
}

void remove_all_listeners() {
  setup();
  ee_t *ee = ee_new();

  ee_on(ee, "hello", on_hello);
  ee_on(ee, "hello", on_hello_again);
  ee_on(ee, "hello", on_hello);
  ee_on(ee, "hello", on_hello);
  t_ok(ee_listener_count(ee, "hello") == 4, "given 4 listeners for 'hello' event");

  ee_on(ee, "hi", on_hi);
  ee_on(ee, "hi", on_hi_again);
  t_ok(ee_listener_count(ee, "hi") == 2, "and 2 listeners for 'hi' event");

  ee_remove_all_listeners(ee, "hello");
  t_ok(ee_listener_count(ee, "hello") == 0, "removing all listeners for 'hello' removes all 'hello' listeners");
  t_ok(ee_listener_count(ee, "hi") == 2, "but none of the 'hi' listeners");

  ee_emit(ee, "hello", "world");
  t_ok(on_hello_count == 0, "emitting 'hello' event afterwards goes unnoticed");
  assert(on_hello_again_count == 0);

  ee_emit(ee, "hi", "world");
  t_ok(on_hi_count == 1, "but emitting 'hi' event triggers handlers");
  assert(on_hi_again_count == 1);

  ee_destroy(ee);
}

int main(void) {
  test(add_listener);
  test(once);
  test(emit);
  test(remove_listener);
  test(remove_all_listeners);

  /* cleanup */
  setup();
}
