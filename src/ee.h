#include <list.h>

#define EE_NEW_LISTENER "new_listener"

#ifndef EE_MAX_LISTENERS
#define EE_MAX_LISTENERS 10
#endif

typedef void (*ee_cb)(void*);
typedef void (*ee_new_listener_cb)(const char*);

typedef struct {
  const char *name;
  ee_cb handler;
} ee_listener_t;

typedef struct {
  const char *name;
  list_t /*<ee_cb>*/ *handlers;
} ee__event_t;

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
