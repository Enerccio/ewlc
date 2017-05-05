#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "internal.h"
#include "context.h"
#include "egl.h"

void*
wlc_context_get_proc_address(struct wlc_context *context, const char *procname)
{
   assert(context && procname);

   if (!context->api.get_proc_address)
      return NULL;

   return context->api.get_proc_address(context->context, procname);
}
bool
wlc_context_query_buffer(struct wlc_context *context, struct wl_resource *buffer, struct wlc_query_buffer_data *data)
{
   assert(context && buffer && data);

   if (!context->api.query_buffer)
      return false;

   return context->api.query_buffer(context->context, buffer, data);
}

void*
wlc_context_create_image(struct wlc_context *context, struct wlc_create_image_data *data)
{
   assert(context && data);

   if (!context->api.create_image)
      return 0;

   return context->api.create_image(context->context, data);
}

bool
wlc_context_destroy_image(struct wlc_context *context, struct wlc_destroy_image_data* data)
{
   assert(context && data);

   if (!context->api.destroy_image)
      return false;

   return context->api.destroy_image(context->context, data);
}

bool
wlc_context_bind(struct wlc_context *context)
{
   assert(context);

   if (!context->api.bind || !context->api.bind(context->context))
      goto fail;

   return true;

fail:
   wlc_log(WLC_LOG_ERROR, "Failed to bind context");
   return false;
}

bool
wlc_context_bind_to_wl_display(struct wlc_context *context, struct wl_display *display)
{
   assert(context);

   if (!context->api.bind_to_wl_display)
      return false;

   return context->api.bind_to_wl_display(context->context, display);
}

void
wlc_context_swap(struct wlc_context *context, struct wlc_backend_surface *bsurface)
{
   assert(context);

   if (context->api.swap)
      context->api.swap(context->context, bsurface);
}

void
wlc_context_release(struct wlc_context *context)
{
   if (!context)
      return;

   if (context->api.terminate)
      context->api.terminate(context->context);

   memset(context, 0, sizeof(struct wlc_context));
}

bool
wlc_context(struct wlc_context *context, struct wlc_backend_surface *surface)
{
   assert(surface);
   memset(context, 0, sizeof(struct wlc_context));

   void* (*constructor[])(struct wlc_backend_surface*, struct wlc_context_api*) = {
      wlc_egl,
      NULL
   };

   for (uint32_t i = 0; constructor[i]; ++i) {
      if ((context->context = constructor[i](surface, &context->api)))
         return true;
   }

   wlc_log(WLC_LOG_WARN, "Could not initialize any context");
   return false;
}
