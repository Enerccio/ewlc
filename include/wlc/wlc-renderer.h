#ifndef _WLC_RENDERER_H_
#define _WLC_RENDERER_H_

#include <stdint.h>
#include <stdbool.h>
#include <wlc/wlc-render.h>
#include "resources/resources.h"

struct wlc_context;
struct wlc_surface;
struct wlc_buffer;
struct wlc_view;
struct wlc_output;
struct wlc_render;
struct wlc_point;
struct wlc_geometry;
struct ctx;

struct wlc_render_api {
   enum wlc_renderer renderer_type;
   WLC_NONULL void (*terminate)(struct ctx *render);
   WLC_NONULL void (*resolution)(struct ctx *render, const struct wlc_size *mode, const struct wlc_size *resolution, uint32_t scale);
   WLC_NONULL void (*surface_destroy)(struct ctx *render, struct wlc_context *bound, struct wlc_surface *surface);
   WLC_NONULLV(1,2,3) bool (*surface_attach)(struct ctx *render, struct wlc_context *bound, struct wlc_surface *surface, struct wlc_buffer *buffer);
   WLC_NONULL void (*view_paint)(struct ctx *render, struct wlc_view *view);
   WLC_NONULL void (*surface_paint)(struct ctx *render, struct wlc_surface *surface, const struct wlc_geometry *geometry);
   WLC_NONULL void (*pointer_paint)(struct ctx *render, const struct wlc_point *pos);
   WLC_NONULL void (*read_pixels)(struct ctx *render, enum wlc_pixel_format format, const struct wlc_geometry *geometry, struct wlc_geometry *out_geometry, void *out_data);
   WLC_NONULL void (*write_pixels)(struct ctx *render, enum wlc_pixel_format format, const struct wlc_geometry *geometry, const void *data);
   WLC_NONULL void (*flush_fakefb)(struct ctx *render);
   WLC_NONULL void (*clear)(struct ctx *render);
};

struct wlc_render {
   void *render; // internal renderer context (OpenGL, etc)
   struct wlc_render_api api;
};

typedef bool (*wlc_renderer_constructor)(struct wlc_render *render, struct wlc_context *context);

bool wlc_output_push_renderer_constructor(wlc_handle handle, wlc_renderer_constructor constructor);


#endif /* _WLC_RENDERER_H_ */
