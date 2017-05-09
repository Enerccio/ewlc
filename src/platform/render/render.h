#ifndef _WLC_RENDER_API_H_
#define _WLC_RENDER_API_H_

#include <wlc/wlc-renderer.h>

WLC_NONULL void wlc_render_resolution(struct wlc_render *render, struct wlc_context *bound, const struct wlc_size *mode, const struct wlc_size *resolution, uint32_t scale);
WLC_NONULL void wlc_render_surface_destroy(struct wlc_render *render, struct wlc_context *bound, struct wlc_surface *surface);
WLC_NONULLV(1,2,3) bool wlc_render_surface_attach(struct wlc_render *render, struct wlc_context *bound, struct wlc_surface *surface, struct wlc_buffer *buffer);
WLC_NONULL void wlc_render_view_paint(struct wlc_render *render, struct wlc_context *bound, struct wlc_view *view);
WLC_NONULL void wlc_render_surface_paint(struct wlc_render *render, struct wlc_context *bound, struct wlc_surface *surface, const struct wlc_geometry *geometry);
WLC_NONULL void wlc_render_pointer_paint(struct wlc_render *render, struct wlc_context *bound, const struct wlc_point *pos);
WLC_NONULL void wlc_render_read_pixels(struct wlc_render *render, struct wlc_context *bound, enum wlc_pixel_format format, const struct wlc_geometry *geometry, struct wlc_geometry *out_geometry, void *out_data);
WLC_NONULL void wlc_render_write_pixels(struct wlc_render *render, struct wlc_context *bound, enum wlc_pixel_format format, const struct wlc_geometry *geometry, const void *data);
WLC_NONULL void wlc_render_flush_fakefb(struct wlc_render *render, struct wlc_context *bound); // only relevant to GLES2
WLC_NONULL void wlc_render_clear(struct wlc_render *render, struct wlc_context *bound);
void wlc_render_release(struct wlc_render *render, struct wlc_context *context);

bool wlc_renderer_empty(struct wlc_render *render, struct wlc_context *context); 

#endif /* _WLC_RENDER_API_H_ */
