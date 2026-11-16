// LD_PRELOAD shim for the Tauri/WebKitGTK build of Plexamp.
//
// Env vars (all optional):
//   PLEXAMP_SMOOTH_SCROLLING=0|1   WebKitSettings:enable-smooth-scrolling (default: leave alone)
//   PLEXAMP_ZOOM=1.25              initial page zoom applied once per webview
//   PLEXAMP_HW_ACCEL=never|always|ondemand   WebKitSettings:hardware-acceleration-policy
//
// Build: make   (see README.md)
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef void *(*get_settings_fn)(void *);
typedef void (*set_bool_fn)(void *, int);
typedef void (*set_int_fn)(void *, int);
typedef void (*set_zoom_fn)(void *, double);

#define MAX_VIEWS 16
static void *seen[MAX_VIEWS];
static int nseen;

static int first_time(void *view) {
    for (int i = 0; i < nseen; i++)
        if (seen[i] == view) return 0;
    if (nseen < MAX_VIEWS) seen[nseen++] = view;
    return 1;
}

void *webkit_web_view_get_settings(void *view) {
    static get_settings_fn real;
    if (!real) real = (get_settings_fn)dlsym(RTLD_NEXT, "webkit_web_view_get_settings");
    void *settings = real(view);
    if (!settings || !first_time(view)) return settings;

    const char *v;
    if ((v = getenv("PLEXAMP_SMOOTH_SCROLLING")) && *v) {
        set_bool_fn f = (set_bool_fn)dlsym(RTLD_NEXT, "webkit_settings_set_enable_smooth_scrolling");
        if (f) f(settings, strcmp(v, "0") != 0);
    }
    if ((v = getenv("PLEXAMP_HW_ACCEL")) && *v) {
        set_int_fn f = (set_int_fn)dlsym(RTLD_NEXT, "webkit_settings_set_hardware_acceleration_policy");
        int p = !strcmp(v, "always") ? 1 : !strcmp(v, "never") ? 2 : 0;
        if (f) f(settings, p);
    }
    if ((v = getenv("PLEXAMP_ZOOM")) && *v) {
        set_zoom_fn f = (set_zoom_fn)dlsym(RTLD_NEXT, "webkit_web_view_set_zoom_level");
        double z = strtod(v, NULL);
        if (f && z > 0.1) f(view, z);
    }
    if (getenv("PLEXAMP_TWEAKS_DEBUG"))
        fprintf(stderr, "[plexamp-tweaks] applied to webview %p\n", view);
    return settings;
}
