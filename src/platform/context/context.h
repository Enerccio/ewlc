#ifndef _WLC_CONTEXT_H_
#define _WLC_CONTEXT_H_

#include <stdbool.h>
#include <wlc/wlc-backend.h>

struct wlc_context {
   void *context; // internal surface context (EGL, etc)
   struct wlc_context_api api;
};

WLC_NONULL void* wlc_context_get_proc_address(struct wlc_context *context, const char *procname);
WLC_NONULL bool wlc_context_query_buffer(struct wlc_context *context, struct wl_resource *buffer, struct wlc_query_buffer_data *data);
WLC_NONULL void* wlc_context_create_image(struct wlc_context *context, struct wlc_create_image_data *data);
WLC_NONULL bool wlc_context_destroy_image(struct wlc_context *context, struct wlc_destroy_image_data* data);
WLC_NONULL bool wlc_context_bind(struct wlc_context *context);
WLC_NONULL bool wlc_context_bind_to_wl_display(struct wlc_context *context, struct wl_display *display);
WLC_NONULL void wlc_context_swap(struct wlc_context *context, struct wlc_backend_surface *bsurface);
void wlc_context_release(struct wlc_context *context);
WLC_NONULL bool wlc_context(struct wlc_context *context, struct wlc_backend_surface *bsurface);

#endif /* _WLC_CONTEXT_H_ */
