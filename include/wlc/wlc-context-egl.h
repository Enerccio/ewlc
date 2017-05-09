#ifndef _WLC_API_CONTEXT_EGL_H
#define _WLC_API_CONTEXT_EGL_H

#include <wlc/wlc-context.h>

#define BACKEND_IDENTIFIER_EGL ("egl")

struct wlc_egl_query_buffer_data {
   struct wlc_query_buffer_data id;
   EGLint attribute;
   EGLint *value;
};

struct wlc_egl_create_image_data {
   struct wlc_create_image_data id;
   EGLenum target;
   EGLClientBuffer buffer;
   const EGLint *attrib_list;
};

struct wlc_egl_destroy_image_data {
   struct wlc_destroy_image_data id;
   EGLImageKHR image;
};

struct wlc_context_api;
struct wlc_backend_surface;

/** 
 * Exposed constructor 
 * You can call it manually from your constructor, should you decide to extend the api.
 **/
bool wlc_egl(struct wlc_context *context, struct wlc_backend_surface *surface);

#endif /* _WLC_API_CONTEXT_EGL_H */
