#include <list.h>

#define EE_NEW_LISTENER "new_listener"

#ifndef EE_MAX_LISTENERS
#define EE_MAX_LISTENERS 10
#endif

typedef void (*ee_cb)(void*);
typedef void (*ee_new_listener_cb)(const char*);

typedef struct {
  const char *name;
  ee_cb cb;
} ee_new_listener_t;

typedef struct {
  ee_cb cb;
  unsigned char once;
} ee_handler_t;

typedef struct {
  list_t /*<ee__event_t>*/ *events;
} ee_t;

ee_t* ee_new                           ( );
void ee_destroy                        ( ee_t* self);

void ee_on                             ( ee_t* self, const char* name, const ee_cb cb);
void ee_add_listener                   ( ee_t* self, const char* name, const ee_cb cb);
void ee_once                           ( ee_t* self, const char* name, const ee_cb cb);

void ee_remove_listener                ( ee_t* self, const char* name, const ee_cb cb);
void ee_remove_all_listeners           ( ee_t* self, const char* name);
void ee_emit                           ( ee_t* self, const char* name, void* arg);

list_t* /*<ee_handler_t>*/ee_listeners ( ee_t* self, const char* name);
int ee_listener_count                  ( ee_t* self, const char* name);
