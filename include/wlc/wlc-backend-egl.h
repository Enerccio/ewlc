#ifndef _WLC_BACKEND_EGL_H_
#define _WLC_BACKEND_EGL_H_

#include <wlc/wlc-backend.h>

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

#endif /* _WLC_BACKEND_EGL_H_ */
