#include <stdio.h>
#include <assert.h>
#include "src/ee.h"

void on_added_new_listener(void* arg) {
  ee_new_listener_t *listener;

  listener = (ee_new_listener_t*) arg;
  printf("New listener added for event '%s'.\n", listener->name);
}

void on_hello(void* arg) {
  char *s = (char*)arg;
  printf("Invoked `on_hello` with  '%s'\n", s);
}

void on_hi(void* arg) {
  char *s = (char*)arg;
  printf("Invoked `on_hi` with  '%s'\n", s);
}

int main(void) {
  const char *event_hello = "hello";
  const char *event_hi = "hi";

  ee_t *ee = ee_new();
  ee_on(ee, EE_NEW_LISTENER, on_added_new_listener);

  assert(0 == ee_listener_count(ee, event_hello));

  /* subscribe "hello" indefinitely, "hi" only once */
  ee_on(ee, event_hello, on_hello);
  ee_once(ee, event_hi, on_hi);

  assert(1 == ee_listener_count(ee, event_hello));
  assert(1 == ee_listener_count(ee, event_hi));

  ee_emit(ee, event_hello, "world");
  ee_emit(ee, event_hi, "world");

  assert(1 == ee_listener_count(ee, event_hello));
  assert(0 == ee_listener_count(ee, event_hi));

  ee_emit(ee, event_hello, "world");
  ee_emit(ee, event_hi, "world"); /* => nothing happens */

  ee_remove_listener(ee, event_hello, on_hello);
  assert(0 == ee_listener_count(ee, event_hello));

  ee_emit(ee, event_hello, "world"); /* => nothing happens */

  return 0;
}
