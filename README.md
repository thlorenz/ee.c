# ee.c [![build status](https://secure.travis-ci.org/thlorenz/ee.c.png)](http://travis-ci.org/thlorenz/ee.c)

EventEmitter in C modeled after [nodejs event emitter](http://nodejs.org/api/events.html).

```c
/* example.c */

#include <stdio.h>
#include <assert.h>
#include "ee.h"

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
```

```
$ make example

New listener added for event 'hello'.
New listener added for event 'hi'.
Invoked `on_hello` with  'world'
Invoked `on_hi` with  'world'
Invoked `on_hello` with  'world'
```

## Installation

    clib install thlorenz/ee.c

## API

Matches API of [nodejs event emitter](http://nodejs.org/api/events.html) so that a large part of its documentation
applies here.

```c
/*
 * Callbacks
 */

/*
 * Callback to register when adding an event listener for all events except EE_NEW_LISTENER.
 */
typedef void (*ee_cb)(void*);

/*
 * Callback to register when adding an event listener for EE_NEW_LISTENER.
 */
typedef void (*ee_new_listener_cb)(ee_new_listener_t* listener);

/*
 * structs
 */

/*
 * new listener struct used as an argument for EE_NEW_LISTENER event
 *
 * @name:  name of event for which a listener was added
 * @cb:    the callback that was registered for that event
 */
struct ee_new_listener_s {
  const char *name;
  ee_cb cb;
};

/*
 * ee struct which stores all information about the event emitter
 *
 * @events:  a list of all registered events with this emitter
 */
typedef struct {
  /* private */
  list_t /*<ee__event_t>*/ *events;
} ee_t;

/*
 * methods
 */

/*
 * create/destroy
 */

/*
 * Initializes new event emitter and returns the result
 */
ee_t* ee_new                           ( );

/*
 * Frees the event emitter
 */
void ee_destroy                        ( ee_t* self);

/*
 * adding listeners
 */

/*
 * Registers the given callback for the event of the given name.
 * Now whenever an event is with that name is emitted, the callback is invoked.
 *
 * @self:  the event emitter
 * @name:  the name of the event we are registering
 * @cb:    the callback to invoke when the event is triggered
 */
void ee_on                             ( ee_t* self, const char* name, const ee_cb cb);

/*
 * Synonym for `on`
 */
void ee_add_listener                   ( ee_t* self, const char* name, const ee_cb cb);

/*
 * Same as `on`, except that the even listener will be automatically removed after the event was emitted once.
 */
void ee_once                           ( ee_t* self, const char* name, const ee_cb cb);

/*
 * removing listeners
 */

/*
 * Removes the given listener for the event with the given name.
 * If the listener was registered more than once, only the first is removed.
 * If the listener was not registered, nothing is done.
 *
 * @self:  the event emitter
 * @name:  the name of the event we are removing the listener for
 * @cb:    the callback to remove
 */
void ee_remove_listener                ( ee_t* self, const char* name, const ee_cb cb);

/*
 * Removes all listeners for the event with the given name.
 * If no listeners for this event were registered, nothing is done.
 *
 * @self:  the event emitter
 * @name:  the name of the event we are removing the listeners for
 */
void ee_remove_all_listeners           ( ee_t* self, const char* name);

/*
 * emitting events
 */

/*
 * Emit an event with the given name and argument
 *
 * @self:  the event emitter
 * @name:  the name of the event we are emitting
 * @arg:   event arg
 */
void ee_emit                           ( ee_t* self, const char* name, void* arg);

/*
 * inspecting listeners
 */

/*
 * Get all listeners for a given event.
 *
 * @self:  the event emitter
 * @name:  the name of the event we are getting listeners of
 * @return a list_t<ee_handler_t> of listeners
 */
list_t* /*<ee_handler_t>*/ ee_listeners ( ee_t* self, const char* name);

/*
 * Get how many listeners are subscribe for a given event
 *
 * @self:  the event emitter
 * @name:  the name of the event we are getting listeners count for
 * @return number of listeners
 */
int ee_listener_count                  ( ee_t* self, const char* name);
```

## License

MIT
