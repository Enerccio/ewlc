#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-server.h>
#include <chck/string/string.h>
#include "internal.h"
#include "macros.h"
#include "egl.h"
#include "context.h"
#include "compositor/compositor.h"
#include "compositor/output.h"
#include "platform/backend/backend.h"

#ifndef EGL_NV_stream_attrib
#define EGL_NV_stream_attrib 1
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLStreamKHR EGLAPIENTRY eglCreateStreamAttribNV(EGLDisplay dpy, const EGLAttrib *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglSetStreamAttribNV(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLAttrib value);
EGLAPI EGLBoolean EGLAPIENTRY eglQueryStreamAttribNV(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLAttrib *value);
EGLAPI EGLBoolean EGLAPIENTRY eglStreamConsumerAcquireAttribNV(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib *attrib_list);
EGLAPI EGLBoolean EGLAPIENTRY eglStreamConsumerReleaseAttribNV(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib *attrib_list);
#endif
typedef EGLStreamKHR (EGLAPIENTRYP PFNEGLCREATESTREAMATTRIBNVPROC)(EGLDisplay dpy, const EGLAttrib *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSETSTREAMATTRIBNVPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLAttrib value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYSTREAMATTRIBNVPROC)(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute, EGLAttrib *value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSTREAMCONSUMERACQUIREATTRIBNVPROC)(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSTREAMCONSUMERRELEASEATTRIBNVPROC)(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib *attrib_list);
#endif /* EGL_NV_stream_attrib */

#ifndef EGL_EXT_stream_acquire_mode
#define EGL_EXT_stream_acquire_mode 1
#define EGL_CONSUMER_AUTO_ACQUIRE_EXT         0x332B
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSTREAMCONSUMERACQUIREATTRIBEXTPROC)(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib *attrib_list);
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglStreamConsumerAcquireAttribEXT(EGLDisplay dpy, EGLStreamKHR stream, const EGLAttrib *attrib_list);
#endif
#endif /* EGL_EXT_stream_acquire_mode */

#ifndef EGL_NV_output_drm_flip_event
#define EGL_NV_output_drm_flip_event 1
#define EGL_DRM_FLIP_EVENT_DATA_NV            0x333E
#endif /* EGL_NV_output_drm_flip_event */

/* XXX khronos eglext.h does not yet have EGL_DRM_MASTER_FD_EXT */
#ifndef EGL_DRM_MASTER_FD_EXT
#define EGL_DRM_MASTER_FD_EXT                   0x333C
#endif

struct ctx {
   const char *extensions;
   const char *device_extensions;
   struct wl_display *wl_display;
   EGLDisplay display;
   EGLContext context;
   EGLSurface surface;
   EGLStreamKHR stream;
   EGLConfig config;
   bool flip_failed;

   struct {
      // Needed for EGL hw surfaces
      PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
      PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
      PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL;
      PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL;
      PFNEGLUNBINDWAYLANDDISPLAYWL eglUnbindWaylandDisplayWL;
      PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC eglSwapBuffersWithDamage;
      // Needed for EGL streams
      PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
      PFNEGLGETOUTPUTLAYERSEXTPROC eglGetOutputLayersEXT;
      PFNEGLCREATESTREAMKHRPROC eglCreateStreamKHR;
      PFNEGLSTREAMCONSUMEROUTPUTEXTPROC eglStreamConsumerOutputEXT;
      PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC eglCreateStreamProducerSurfaceKHR;
      PFNEGLSTREAMCONSUMERACQUIREATTRIBNVPROC eglStreamConsumerAcquireAttribNV;
   } api;
};

static const char*
egl_error_string(const EGLint error)
{
   switch (error) {
      case EGL_SUCCESS:
         return "Success";
      case EGL_NOT_INITIALIZED:
         return "EGL is not or could not be initialized";
      case EGL_BAD_ACCESS:
         return "EGL cannot access a requested resource";
      case EGL_BAD_ALLOC:
         return "EGL failed to allocate resources for the requested operation";
      case EGL_BAD_ATTRIBUTE:
         return "An unrecognized attribute or attribute value was passed "
                "in the attribute list";
      case EGL_BAD_CONTEXT:
         return "An EGLContext argument does not name a valid EGL "
                "rendering context";
      case EGL_BAD_CONFIG:
         return "An EGLConfig argument does not name a valid EGL frame "
                "buffer configuration";
      case EGL_BAD_CURRENT_SURFACE:
         return "The current surface of the calling thread is a window, pixel "
                "buffer or pixmap that is no longer valid";
      case EGL_BAD_DISPLAY:
         return "An EGLDisplay argument does not name a valid EGL display "
                "connection";
      case EGL_BAD_SURFACE:
         return "An EGLSurface argument does not name a valid surface "
                "configured for GL rendering";
      case EGL_BAD_MATCH:
         return "Arguments are inconsistent";
      case EGL_BAD_PARAMETER:
         return "One or more argument values are invalid";
      case EGL_BAD_NATIVE_PIXMAP:
         return "A NativePixmapType argument does not refer to a valid "
                "native pixmap";
      case EGL_BAD_NATIVE_WINDOW:
         return "A NativeWindowType argument does not refer to a valid "
                "native window";
      case EGL_CONTEXT_LOST:
         return "The application must destroy all contexts and reinitialise";
   }

   return "UNKNOWN EGL ERROR";
}

static void
egl_call(const char *func, uint32_t line, const char *eglfunc)
{
   EGLint error;
   if ((error = eglGetError()) == EGL_SUCCESS)
      return;

   wlc_log(WLC_LOG_ERROR, "egl: function %s at line %u: %s\n%s", func, line, eglfunc, egl_error_string(error));
}

#ifndef __STRING
#  define __STRING(x) #x
#endif

#define EGL_CALL(x) x; egl_call(__PRETTY_FUNCTION__, __LINE__, __STRING(x))

WLC_PURE static bool
is_extension_supported(const char *extensions, const char *extension)
{
   assert(extension);

   if (!extensions)
      return false;

   size_t len = strlen(extension), pos;
   const char *s = extensions;
   while ((pos = strcspn(s, " ")) != 0) {
      size_t next = pos + (s[pos] != 0);

      if (chck_cstrneq(s, extension, len))
         return true;

      s += next;
   }

   return false;
}

WLC_PURE static bool
has_extension(const struct ctx *context, const char *extension)
{
   assert(context && extension);
   return is_extension_supported(context->extensions, extension);
}

WLC_PURE static bool
has_device_extension(const struct ctx *context, const char *extension)
{
   assert(context && extension);
   return is_extension_supported(context->device_extensions, extension);
}

EGLDeviceEXT
get_egl_device()
{
   PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT = (void*)eglGetProcAddress("eglQueryDevicesEXT");
   PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT = (void*)eglGetProcAddress("eglQueryDeviceStringEXT");

   const char *extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
   if (!is_extension_supported(extensions, "EGL_EXT_device_base") &&
       (!is_extension_supported(extensions, "EGL_EXT_device_enumeration") ||
        !is_extension_supported(extensions, "EGL_EXT_device_query")))
      return NULL;

   EGLint num_devices;
   if (!eglQueryDevicesEXT(0, NULL, &num_devices) || num_devices < 1)
      return NULL;

   EGLDeviceEXT devices[255];
   if (!eglQueryDevicesEXT(num_devices, devices, &num_devices))
      return NULL;

   for (int i = 0; i < num_devices; i++) {
      EGLDeviceEXT device = devices[i];
      const char *device_extensions = eglQueryDeviceStringEXT(device, EGL_EXTENSIONS);
      if (is_extension_supported(device_extensions, "EGL_EXT_device_drm") && device != EGL_NO_DEVICE_EXT)
         return device;
   }

   return NULL;
}

static void
terminate(struct ctx *context)
{
   assert(context);

   EGL_CALL(eglMakeCurrent(context->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

   if (context->surface) {
      EGL_CALL(eglDestroySurface(context->display, context->surface));
   }

   if (context->context) {
      EGL_CALL(eglDestroyContext(context->display, context->context));
   }

   // XXX: This is shared on all backends
#if 0
   if (context->display) {
      if (context->api.eglUnbindWaylandDisplayWL && context->wl_display) {
         EGL_CALL(context->api.eglUnbindWaylandDisplayWL(context->display, context->wl_display));
      }

      EGL_CALL(eglTerminate(context->display));
   }
#endif

   free(context);
}

/*
 * Taken from glamor_egl.c from the Xorg xserver, which is MIT licensed like wlc
 *
 * Create an EGLDisplay from a native display type. This is a little quirky
 * for a few reasons.
 *
 * 1: GetPlatformDisplayEXT and GetPlatformDisplay are the API you want to
 * use, note these have different function signatures in the third argument.
 *
 * 2: You can't tell whether you have EGL 1.5 at this point, because
 * eglQueryString(EGL_VERSION) is a property of the display, which we don't
 * have yet. So you have to query for extensions no matter what.
 *
 * 3. There is no EGL_KHR_platform_base to complement the EXT one, thus one
 * needs to know EGL 1.5 is supported in order to use the eglGetPlatformDisplay
 * function pointer.
 *
 * We can workaround this (circular dependency) by probing for the EGL 1.5
 * platform extensions (EGL_KHR_platform_gbm and friends) yet it doesn't seem
 * like mesa will be able to adverise these (even though it can do EGL 1.5).
 */
static EGLDisplay
get_display(struct ctx *context, EGLint type, void *native, int drm_fd)
{
   /* In practice any EGL 1.5 implementation would support the EXT extension */
   if (context->api.eglGetPlatformDisplayEXT) {
      if (has_extension(context, "EGL_EXT_platform_device") && has_device_extension(context, "EGL_EXT_device_drm")) {
         /*
          * Provide the DRM fd when creating the EGLDisplay, so that the
          * EGL implementation can make any necessary DRM calls using the
          * same fd as the application.
          */
         EGLint attribs[] = {
            EGL_DRM_MASTER_FD_EXT, drm_fd,
            EGL_NONE
         };

         return context->api.eglGetPlatformDisplayEXT(type, native, attribs);
      }

      return context->api.eglGetPlatformDisplayEXT(type, native, NULL);
   }

   /* Welp, everything is awful. */
   return eglGetDisplay(native);
}

static EGLSurface
create_surface_egl_device(struct ctx *context, struct wlc_backend_surface *bsurface)
{
   struct wlc_output *output;
   output = wl_container_of(bsurface, output, bsurface);

   EGLint n = 0;
   EGLOutputLayerEXT egl_layer;
   EGLAttrib layer_attribs[] = {
      EGL_DRM_CRTC_EXT, output->information.crtc_id,
      EGL_NONE,
   };

   if (!context->api.eglGetOutputLayersEXT(context->display, layer_attribs, &egl_layer, 1, &n) || !n) {
      return EGL_NO_SURFACE;
   }

   if (!context->api.eglStreamConsumerOutputEXT(context->display, context->stream, egl_layer)) {
      return EGL_NO_SURFACE;
   }

   EGLint surface_attribs[] = {
      EGL_WIDTH, output->mode.w,
      EGL_HEIGHT, output->mode.h,
      EGL_NONE
   };

   return context->api.eglCreateStreamProducerSurfaceKHR(context->display, context->config, context->stream, surface_attribs);
}

static EGLSurface
create_surface_gbm(EGLDisplay display, EGLConfig config, EGLNativeWindowType window)
{
   return eglCreateWindowSurface(display, config, window, NULL);
}

static struct ctx*
create_context(struct wlc_backend_surface *bsurface)
{
   assert(bsurface);

   struct ctx *context;
   if (!(context = calloc(1, sizeof(struct ctx))))
      return NULL;

   /* Initialize extensions to those of the NULL display, for has_extension */
   context->extensions = EGL_CALL(eglQueryString(NULL, EGL_EXTENSIONS));
   if (has_extension(context, "EGL_EXT_platform_base")) {
      context->api.eglGetPlatformDisplayEXT = (void*)eglGetProcAddress("eglGetPlatformDisplayEXT");
   }

   if (bsurface->use_egldevice) {
      PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT = (void*)eglGetProcAddress("eglQueryDeviceStringEXT");
      context->device_extensions = EGL_CALL(eglQueryDeviceStringEXT(bsurface->display, EGL_EXTENSIONS));

      context->api.eglGetOutputLayersEXT = (void*)eglGetProcAddress("eglGetOutputLayersEXT");
      context->api.eglCreateStreamKHR = (void*)eglGetProcAddress("eglCreateStreamKHR");
      context->api.eglStreamConsumerOutputEXT = (void*)eglGetProcAddress("eglStreamConsumerOutputEXT");
      context->api.eglCreateStreamProducerSurfaceKHR = (void*)eglGetProcAddress("eglCreateStreamProducerSurfaceKHR");
      context->api.eglStreamConsumerAcquireAttribNV = (void*)eglGetProcAddress("eglStreamConsumerAcquireAttribNV");
   }

   context->display = get_display(context, bsurface->display_type, bsurface->display, bsurface->drm_fd);
   if (!context->display)
      goto egl_fail;

   EGLint major, minor;
   if (!eglInitialize(context->display, &major, &minor))
      goto egl_fail;

   if (!eglBindAPI(EGL_OPENGL_ES_API))
      goto egl_fail;

   const struct {
      const EGLint *attribs;
   } configs[] = {
      {
         (const EGLint[]){
            EGL_SURFACE_TYPE, bsurface->use_egldevice ? EGL_STREAM_BIT_KHR : EGL_WINDOW_BIT,
            EGL_RED_SIZE, 1,
            EGL_GREEN_SIZE, 1,
            EGL_BLUE_SIZE, 1,
            EGL_ALPHA_SIZE, 0,
            EGL_DEPTH_SIZE, 1,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
         }
      }
   };

   for (uint32_t i = 0; i < LENGTH(configs); ++i) {
      EGLint n;
      if (eglChooseConfig(context->display, configs[i].attribs, &context->config, 1, &n) && n > 0)
         break;

      context->config = NULL;
   }

   if (!context->config)
      goto egl_fail;

   const EGLint context_attribs[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   if ((context->context = eglCreateContext(context->display, context->config, EGL_NO_CONTEXT, context_attribs)) == EGL_NO_CONTEXT)
      goto egl_fail;

   if (bsurface->use_egldevice) {
      EGLint stream_attribs[] = {
         EGL_STREAM_FIFO_LENGTH_KHR, 1,
         EGL_CONSUMER_AUTO_ACQUIRE_EXT, EGL_FALSE,
         EGL_NONE
      };

      context->stream = context->api.eglCreateStreamKHR(context->display, stream_attribs);
      if (context->stream == EGL_NO_STREAM_KHR)
         goto egl_fail;
   }

   context->surface = bsurface->use_egldevice ? create_surface_egl_device(context, bsurface) : create_surface_gbm(context->display, context->config, bsurface->window);
   if (context->surface == EGL_NO_SURFACE)
      goto egl_fail;

   if (!eglMakeCurrent(context->display, context->surface, context->surface, context->context))
      goto egl_fail;

   EGLint render_buffer;
   if (!eglQueryContext(context->display, context->context, EGL_RENDER_BUFFER, &render_buffer))
      goto egl_fail;

   switch (render_buffer) {
      case EGL_SINGLE_BUFFER:
         wlc_log(WLC_LOG_INFO, "EGL context is single buffered");
         break;
      case EGL_BACK_BUFFER:
         wlc_log(WLC_LOG_INFO, "EGL context is double buffered");
         break;
      default: break;
   }

   const char *str;
   str = EGL_CALL(eglQueryString(context->display, EGL_VERSION));
   wlc_log(WLC_LOG_INFO, "EGL version: %s", str ? str : "(null)");
   str = EGL_CALL(eglQueryString(context->display, EGL_VENDOR));
   wlc_log(WLC_LOG_INFO, "EGL vendor: %s", str ? str : "(null)");
   str = EGL_CALL(eglQueryString(context->display, EGL_CLIENT_APIS));
   wlc_log(WLC_LOG_INFO, "EGL client APIs: %s", str ? str : "(null)");

   {
      struct {
         EGLint r, g, b, a;
      } config;

      EGL_CALL(eglGetConfigAttrib(context->display, context->config, EGL_RED_SIZE, &config.r));
      EGL_CALL(eglGetConfigAttrib(context->display, context->config, EGL_GREEN_SIZE, &config.g));
      EGL_CALL(eglGetConfigAttrib(context->display, context->config, EGL_BLUE_SIZE, &config.b));
      EGL_CALL(eglGetConfigAttrib(context->display, context->config, EGL_ALPHA_SIZE, &config.a));

      if (config.a > 0) {
         wlc_log(WLC_LOG_INFO, "EGL context (RGBA%d%d%d%d)", config.r, config.g, config.b, config.a);
      } else {
         wlc_log(WLC_LOG_INFO, "EGL context (RGB%d%d%d)", config.r, config.g, config.b);
      }
   }

   context->extensions = EGL_CALL(eglQueryString(context->display, EGL_EXTENSIONS));
   wlc_log(WLC_LOG_INFO, "%s", context->extensions);

   if (has_extension(context, "EGL_WL_bind_wayland_display") && has_extension(context, "EGL_KHR_image_base")) {
      context->api.eglCreateImageKHR = (void*)eglGetProcAddress("eglCreateImageKHR");
      context->api.eglDestroyImageKHR = (void*)eglGetProcAddress("eglDestroyImageKHR");
      context->api.eglBindWaylandDisplayWL = (void*)eglGetProcAddress("eglBindWaylandDisplayWL");
      context->api.eglUnbindWaylandDisplayWL = (void*)eglGetProcAddress("eglUnbindWaylandDisplayWL");
      context->api.eglQueryWaylandBufferWL = (void*)eglGetProcAddress("eglQueryWaylandBufferWL");
   }

   if (has_extension(context, "EGL_EXT_swap_buffers_with_damage")) {
      // FIXME: get hw that supports this feature
      context->api.eglSwapBuffersWithDamage = (void*)eglGetProcAddress("eglSwapBuffersWithDamage");
   } else {
      // FIXME: get hw that supports this feature
      // wlc_log(WLC_LOG_WARN, "EGL_EXT_swap_buffers_with_damage not supported. Performance could be affected.");
   }

   EGL_CALL(eglSwapInterval(context->display, 1));
   return context;

egl_fail:
   {
      EGLint error;
      if ((error = eglGetError()) != EGL_SUCCESS)
         wlc_log(WLC_LOG_WARN, "%s", egl_error_string(error));
   }
   terminate(context);
   return NULL;
}

static bool
bind(struct ctx *context)
{
   static struct ctx *bound = NULL;
   assert(context);

   if (context == bound)
      return true;

   EGLBoolean made_current = EGL_CALL(eglMakeCurrent(context->display, context->surface, context->surface, context->context));
   if (made_current != EGL_TRUE)
      return false;

   bound = context;
   return true;
}

static bool
bind_to_wl_display(struct ctx *context, struct wl_display *wl_display)
{
   assert(context);

   const char *env;
   if ((env = getenv("WLC_SHM")) && chck_cstreq(env, "1"))
      return false;

   if (context->api.eglBindWaylandDisplayWL) {
      EGLBoolean binded = EGL_CALL(context->api.eglBindWaylandDisplayWL(context->display, wl_display));
      if (binded == EGL_TRUE)
         context->wl_display = wl_display;
   }

   return (context->wl_display ? true : false);
}

static bool
output_stream_flip(struct ctx *context, struct wlc_backend_surface *bsurface)
{
   EGLAttrib acquire_attribs[] = {
      EGL_DRM_FLIP_EVENT_DATA_NV, (EGLAttrib)bsurface,
      EGL_NONE
   };

   if (context->stream != EGL_NO_STREAM_KHR) {
      return context->api.eglStreamConsumerAcquireAttribNV(context->display, context->stream, acquire_attribs) == EGL_TRUE;
   }

   return true;
}

static void
swap(struct ctx *context, struct wlc_backend_surface *bsurface)
{
   assert(context);

   EGLBoolean ret = EGL_FALSE;

   if (!bind(context)) {
      wlc_log(WLC_LOG_ERROR, "Failed to bind context.");
      abort();
   }

   if (!context->flip_failed)
      ret = EGL_CALL(eglSwapBuffers(context->display, context->surface));

   if (ret == EGL_TRUE && bsurface->use_egldevice) {
      output_stream_flip(context, bsurface);
   } else if (ret == EGL_TRUE && bsurface->api.page_flip)
      context->flip_failed = !bsurface->api.page_flip(bsurface);
}

static void*
get_proc_address(struct ctx *context, const char *procname)
{
   (void)context;
   assert(context && procname);
   return eglGetProcAddress(procname);
}

static bool
query_buffer(struct ctx *context, struct wl_resource *buffer, struct wlc_query_buffer_data *data)
{
   assert(context);
   if (context->api.eglQueryWaylandBufferWL) {
      assert(strcmp(data->identifier, BACKEND_IDENTIFIER_EGL) == 0); // must be egl backend
      struct wlc_egl_query_buffer_data *args = (struct wlc_egl_query_buffer_data *)data;
      
      return EGL_CALL(context->api.eglQueryWaylandBufferWL(context->display, 
         buffer, args->attribute, args->value));
   }
   return EGL_FALSE;
}

static void*
create_image(struct ctx *context, struct wlc_create_image_data *data)
{
   assert(context);
   if (context->api.eglCreateImageKHR) {
      assert(strcmp(data->identifier, BACKEND_IDENTIFIER_EGL) == 0); // must be egl backend
      struct wlc_egl_create_image_data *args = (struct wlc_egl_create_image_data *)data;
      
      return (void*)EGL_CALL(context->api.eglCreateImageKHR(context->display, context->context, 
         args->target, args->buffer, args->attrib_list));
   }
   return NULL;
}

static bool
destroy_image(struct ctx *context, struct wlc_destroy_image_data *data)
{
   assert(context && data);
   if (context->api.eglDestroyImageKHR) {
      assert(strcmp(data->identifier, BACKEND_IDENTIFIER_EGL) == 0); // must be egl backend
      return EGL_CALL(context->api.eglDestroyImageKHR(context->display, 
         ((struct wlc_egl_destroy_image_data*)data)->image));
   }
   return EGL_FALSE;
}

bool 
wlc_egl(struct wlc_context *context, struct wlc_backend_surface *bsurface) 
{
   assert(context && bsurface);
   memset(context, 0, sizeof(struct wlc_context));
   
   struct wlc_context_api *api = &context->api;
   
   struct ctx *ctx;
   if (!(ctx = create_context(bsurface)))
      return false;

   api->terminate = terminate;
   api->bind = bind;
   api->bind_to_wl_display = bind_to_wl_display;
   api->swap = swap;
   api->get_proc_address = get_proc_address;
   api->destroy_image = destroy_image;
   api->create_image = create_image;
   api->query_buffer = query_buffer;
   
   context->context = ctx;
   
   return true;
}
