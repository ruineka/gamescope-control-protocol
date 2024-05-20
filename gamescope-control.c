#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include "gamescope-control-client-protocol.h"
#include <stdbool.h>

// Interface functions to call from gamescope
static void gamescope_control_handle_destroy(struct wl_client *client, struct wl_resource *resource);
void gamescope_control_set_app_target_refresh_cycle(struct gamescope_control *gamescope_control, uint32_t fps, uint32_t flags);
static void takeScreenshot(struct gamescope_control *gamescope_control, const char *path, uint32_t type, uint32_t flags);
static void rotateDisplay(struct gamescope_control *gamescope_control, uint32_t orientation, uint32_t target_type);
/*
static void forcePanelType(struct gamescope_control *gamescope_control, const char *panel_type);
static void targetConnector(struct gamescope_control *gamescope_control, uint32_t connector_id);
*/
static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t id,
                                   const char *interface, uint32_t version) {
    struct wl_display *display = data;

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

            // Send requests
            //takeScreenshot(myGamescopeControl, "/home/ruineka/gamescope.png", GAMESCOPE_CONTROL_SCREENSHOT_TYPE_BASE_PLANE_ONLY, 0);
            rotateDisplay(myGamescopeControl, 1, 1); // Set as version 2 on the server so it works..

            // Cleanup: Destroy the gamescope_control object
            wl_proxy_destroy((struct wl_proxy *)myGamescopeControl);
        } else {
            fprintf(stderr, "Failed to bind to gamescope_control interface\n");
        }
    }
}

static void takeScreenshot(struct gamescope_control *gamescope_control, const char *path, uint32_t type, uint32_t flags) {
    gamescope_control_take_screenshot(gamescope_control, path, type, flags);
}

static void rotateDisplay(struct gamescope_control *gamescope_control, uint32_t orientation, uint32_t target_type) {
    gamescope_control_rotate_display(gamescope_control, orientation, target_type);
}

/*static void targetConnector(struct gamescope_control *gamescope_control, uint32_t connector_type) {
    gamescope_control_target_connector(gamescope_control, connector_type);
}

static void forcePanelType(struct gamescope_control *gamescope_control, const char *panel_type) {
    gamescope_control_force_panel_type(gamescope_control, panel_type);
}*/

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <display-number>\n", argv[0]);
        return 1;
    }

    const char *display_number_str = argv[1];
    char socket_path[256];

    snprintf(socket_path, sizeof(socket_path), "/run/user/%d/gamescope-%s", getuid(), display_number_str); // Be sure to give the number of the socket..for example gamescope-0 gamescope-1 etc

    struct wl_display *display = wl_display_connect(socket_path);

    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display at %s\n", socket_path);
        return 1;
    }

    printf("Connected to Wayland display at %s\n", socket_path);

    struct wl_registry *registry = wl_display_get_registry(display);

    struct wl_registry_listener registry_listener = {
        .global = registry_handle_global,
    };

    wl_registry_add_listener(registry, &registry_listener, display);
    wl_display_roundtrip(display);

    // Run the Wayland event loop
    while (wl_display_dispatch(display) != -1) {
        // Continue processing events
    }

    wl_display_disconnect(display);

    return 0;
}

