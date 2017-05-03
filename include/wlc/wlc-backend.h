#ifndef _WLC_API_BACKEND_H_
#define _WLC_API_BACKEND_H_

#include <stdbool.h>
#include <wlc/defines.h>
#include <wlc/geometry.h>
#include <wlc/wlc-wayland.h>

struct wl_display;
struct wlc_backend_surface;
struct ctx;

typedef const char* wlc_backend_identifier;

struct wlc_query_buffer_data {
   wlc_backend_identifier identifier;
};

struct wlc_create_image_data {
   wlc_backend_identifier identifier;
};

struct wlc_destroy_image_data {
   wlc_backend_identifier identifier;
};

struct wlc_context_api {
   WLC_NONULL void (*terminate)(struct ctx *context);
   WLC_NONULL bool (*bind)(struct ctx *context);
   WLC_NONULL bool (*bind_to_wl_display)(struct ctx *context, struct wl_display *display);
   WLC_NONULL void (*swap)(struct ctx *context, struct wlc_backend_surface *bsurface);
   WLC_NONULL void* (*get_proc_address)(struct ctx *context, const char *procname);
   WLC_NONULL bool (*query_buffer)(struct ctx *context, struct wl_resource *buffer, struct wlc_query_buffer_data *data);
   WLC_NONULL void* (*create_image)(struct ctx *context, struct wlc_create_image_data *data);
   WLC_NONULL bool (*destroy_image)(struct ctx *context, struct wlc_destroy_image_data* data);
};

#endif /* _WLC_API_BACKEND_H_ */
