#ifndef _WLC_VIEW_H_
#define _WLC_VIEW_H_

#include <stdbool.h>
#include <sys/types.h>
#include <wlc/geometry.h>
#include <wayland-util.h>
#include <chck/pool/pool.h>
#include <chck/string/string.h>
#include "xwayland/xwm.h"
#include "resources/resources.h"

enum output_link;
struct wlc_surface;
struct wlc_x11_window;

enum wlc_view_ack {
   ACK_NONE,
   ACK_PENDING,
   ACK_NEXT_COMMIT
};

struct wlc_view_state {
   struct wlc_geometry geometry;
   uint32_t edges, state;
};

struct wlc_view_surface_state {
   struct wlc_geometry visible;
};

struct wlc_view {
   struct wlc_x11_window x11;
   struct wlc_view_state pending;
   struct wlc_view_state commit;
   struct wlc_view_surface_state surface_pending;
   struct wlc_view_surface_state surface_commit;
   struct chck_iter_pool wl_state;

   wlc_handle parent;
   wlc_resource surface;
   wlc_resource shell_surface;
   wlc_resource xdg_surface;
   wlc_resource xdg_toplevel;
   wlc_resource xdg_popup;
   wlc_resource custom_surface;

   struct {
      struct chck_string app_id;
      struct chck_string title;
      struct chck_string _instance;
      struct chck_string _class;
      pid_t pid;
      enum wl_shell_surface_fullscreen_method fullscreen_mode;
      bool minimized;
      enum wlc_view_properties props;
   } data;

   uint32_t type;
   uint32_t mask;

   struct {
      bool created;
   } state;
};

static inline bool
is_x11_view(struct wlc_view *view)
{
   return wlc_x11_is_valid_window(&view->x11);
}

WLC_NONULL void wlc_view_update(struct wlc_view *view);
WLC_NONULL void wlc_view_map(struct wlc_view *view);
WLC_NONULL void wlc_view_unmap(struct wlc_view *view);
WLC_NONULL void wlc_view_commit_state(struct wlc_view *view, struct wlc_view_state *pending, struct wlc_view_state *out);
WLC_NONULL void wlc_view_ack_surface_attach(struct wlc_view *view, struct wlc_surface *surface);
WLC_NONULLV(1,2) void wlc_view_get_bounds(struct wlc_view *view, struct wlc_geometry *out_bounds, struct wlc_geometry *out_visible);
WLC_NONULL bool wlc_view_get_opaque(struct wlc_view *view, struct wlc_geometry *out_opaque);
WLC_NONULL void wlc_view_get_input(struct wlc_view *view, struct wlc_geometry *out_input);
WLC_NONULL bool wlc_view_request_geometry(struct wlc_view *view, const struct wlc_geometry *r);
bool wlc_view_request_state(struct wlc_view *view, enum wlc_view_state_bit state, bool toggle);
void wlc_view_set_surface(struct wlc_view *view, struct wlc_surface *surface);
struct wl_client* wlc_view_get_client_ptr(struct wlc_view *view);
void wlc_view_release(struct wlc_view *view);
WLC_NONULL bool wlc_view(struct wlc_view *view);

void wlc_view_send_to_other(struct wlc_view *view, enum output_link link, struct wlc_view *other);
void wlc_view_send_to(struct wlc_view *view, enum output_link link);
void wlc_view_focus_ptr(struct wlc_view *view);
void wlc_view_close_ptr(struct wlc_view *view);
struct wlc_output* wlc_view_get_output_ptr(struct wlc_view *view);
void wlc_view_set_output_ptr(struct wlc_view *view, struct wlc_output *output);
void wlc_view_send_to_back_ptr(struct wlc_view *view);
void wlc_view_send_below_ptr(struct wlc_view *view, struct wlc_view *other);
void wlc_view_bring_above_ptr(struct wlc_view *view, struct wlc_view *other);
void wlc_view_bring_to_front_ptr(struct wlc_view *view);
void wlc_view_set_mask_ptr(struct wlc_view *view, uint32_t mask);
WLC_NONULLV(3) void wlc_view_set_geometry_ptr(struct wlc_view *view, uint32_t edges, const struct wlc_geometry *geometry);
void wlc_view_set_type_ptr(struct wlc_view *view, enum wlc_view_type_bit type, bool toggle);
void wlc_view_set_state_ptr(struct wlc_view *view, enum wlc_view_state_bit state, bool toggle);
void wlc_view_set_parent_ptr(struct wlc_view *view, struct wlc_view *parent);
void wlc_view_set_minimized_ptr(struct wlc_view *view, bool minimized);
void wlc_view_set_title_ptr(struct wlc_view *view, const char *title, size_t length);
void wlc_view_set_instance_ptr(struct wlc_view *view, const char *instance_, size_t length);
void wlc_view_set_class_ptr(struct wlc_view *view, const char *class_, size_t length);
void wlc_view_set_app_id_ptr(struct wlc_view *view, const char *app_id);
void wlc_view_set_pid_ptr(struct wlc_view *view, pid_t pid);
bool wlc_view_is_minimized_ptr(struct wlc_view *view);
enum wlc_x11_window_type wlc_view_x11_get_type_ptr(struct wlc_view *view);
enum wlc_view_properties wlc_view_get_properties_ptr(struct wlc_view *view);

#endif /* _WLC_VIEW_H_ */
