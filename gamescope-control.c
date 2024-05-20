#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include "gamescope-control-client-protocol.h"
#include <unistd.h>

static int exit_event_loop = 0;

// Struct to hold display data
struct DisplayData {
    struct wl_display *display;
    uint32_t arg1;
    uint32_t arg2;
};

// Callback listener
static void callback_done(void *data, struct wl_callback *callback, uint32_t time) {
    printf("Request completed successfully at time %u\n", time);
    wl_callback_destroy(callback);
    exit_event_loop = 1;
}

static const struct wl_callback_listener callback_listener = {
    callback_done
};

// Registry handler
static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t id,
                                   const char *interface, uint32_t version) {
    struct DisplayData *display_data = (struct DisplayData *)data;

    if (strcmp(interface, "gamescope_control") == 0) {
        // Bind to the gamescope_control interface
        struct wl_proxy *gamescope_control_proxy =
        wl_registry_bind(registry, id, &gamescope_control_interface, version);

        // Check if binding was successful
        if (gamescope_control_proxy) {
            // Now you have the gamescope_control object
            // You can set up listeners, send requests, etc.
            struct gamescope_control *myGamescopeControl = (struct gamescope_control *)gamescope_control_proxy;
            printf("Successfully bound to gamescope_control\n");

            // Send requests (This will be made dynamic in the future and won't be hard coded)
            gamescope_control_rotate_display(myGamescopeControl, display_data->arg1, display_data->arg2);

            // We need to listen for the callback to confirm the command was received successfully
            struct wl_callback *callback = wl_display_sync(display_data->display);
            wl_callback_add_listener(callback, &callback_listener, NULL);

            // Destroy the gamescope_control object after the request is processed
            wl_proxy_destroy((struct wl_proxy *)myGamescopeControl);
        } else {
            fprintf(stderr, "Failed to bind to gamescope_control interface\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <display-number> <orientation> <target-type>\n", argv[0]);
        return 1;
    }

    const char *display_number_str = argv[1];
    uint32_t arg1 = atoi(argv[2]);
    uint32_t arg2 = atoi(argv[3]);
    //uint32_t arg3 - atoi(argv[4];) Eventually we will target what function we want by passing a parameter

    char socket_path[256];
    snprintf(socket_path, sizeof(socket_path), "/run/user/%d/gamescope-%s", getuid(), display_number_str);

    struct wl_display *display = wl_display_connect(socket_path);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display at %s\n", socket_path);
        return 1;
    }

    printf("Connected to Wayland display at %s\n", socket_path);

    struct wl_registry *registry = wl_display_get_registry(display);
    struct DisplayData display_data = { display, arg1, arg2 };

    struct wl_registry_listener registry_listener = {
        .global = registry_handle_global,
    };

    wl_registry_add_listener(registry, &registry_listener, &display_data);
    wl_display_roundtrip(display);

    // See here https://wayland-book.com/wayland-display/event-loop.html
    // Run the Wayland event loop
    while (!exit_event_loop && wl_display_dispatch(display) != -1) {
        // Continue processing events
    }

    wl_display_disconnect(display);

    return 0;
}
