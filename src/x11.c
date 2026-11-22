#include "x11.h"

void x11_init(X11Context *ctx) {
    Display* disp = XOpenDisplay(NULL);

    if(disp == NULL) {
        fprintf(stderr, "error: Unable to connect to X11\n");
        exit(1);
    }

    int opcode;
    int query_event;
    int query_error;

    if(!XQueryExtension(disp, "XInputExtension", &opcode, &query_event, &query_error)) {
        fprintf(stderr, "error: Failed to query X input extension. Error code: %d\n", query_error);
        exit(1);
    }

    if (!XScreenSaverQueryExtension(disp, &query_event, &query_error)) {
        fprintf(stderr, "error: Failed to query X11 screensaver extension. Error code: %d\n", query_error);
        exit(1);
    }

    XIEventMask mask;
    unsigned char mask_data[XIMaskLen(XI_LASTEVENT)] = {0};

    XISetMask(mask_data, XI_KeyPress);
    XISetMask(mask_data, XI_KeyRelease);
    XISetMask(mask_data, XI_Motion);
    
    mask.deviceid = XIAllDevices;
    mask.mask_len = sizeof(mask_data);
    mask.mask = mask_data;

    XISelectEvents(disp, DefaultRootWindow(disp), &mask, 1);
    XFlush(disp);

    ctx->disp = disp;
    ctx->socket = ConnectionNumber(disp);
    
    ctx->event_timeout_timer.tv_sec = 0;
    ctx->event_timeout_timer.tv_usec = 5000;
}

bool x11_has_interacted(X11Context *ctx) {
    FD_ZERO(&ctx->event_fd);
    FD_SET(ctx->socket, &ctx->event_fd);

    int event_count = select(
            ctx->socket + 1, 
            &ctx->event_fd, 
            NULL,
            NULL,
            &ctx->event_timeout_timer
    );

    if(event_count == 0) {
        return false;
    }

    bool res = false;

    // just flush the events
    while(XPending(ctx->disp)) {
        XEvent ev;
        XNextEvent(ctx->disp, &ev);
        res = true;
    }

    return res;
}

bool x11_is_window_fullscreen(X11Context *ctx, Window w) {
    Window root = DefaultRootWindow(ctx->disp);

    XWindowAttributes root_attr = {0};
    XWindowAttributes attr = {0};
    
    if(XGetWindowAttributes(ctx->disp, root, &root_attr) == 0) {
        return false;
    }

    if(XGetWindowAttributes(ctx->disp, w, &attr) == 0) {
        return false;
    }

    if(attr.height == root_attr.height) {
        return true;
    }

    return false;
}

bool x11_has_screensaver(X11Context *ctx) {
    XScreenSaverInfo *info = XScreenSaverAllocInfo();

    if(info == NULL) {
        fprintf(stderr, "error: Failed to allocate screen saver info\n");
    }

    bool res = false;

    if(XScreenSaverQueryInfo(ctx->disp, DefaultRootWindow(ctx->disp), info)) {
        res = info->state == ScreenSaverOn;
    }

    XFree(info);

    return res;
}

bool x11_has_lockscreen(X11Context *ctx) {
    Window root = XDefaultRootWindow(ctx->disp);

    Window parent;

    Window *windows;
    unsigned int windows_len;

    int status = XQueryTree(
        ctx->disp, 
        root,
        &root, // root
        &parent,  // parent
        &windows, 
        &windows_len
    );
    
    if(status == 0) {
        fprintf(stderr, "error: Failed to list windows.\n");
        return false;
    }

    if(windows == NULL) {
        fprintf(stderr, "error: Failed to list windows. Array is NULL.\n");
        return false;
    }

    for(size_t i = 0; i < windows_len; i++) {
        XWindowAttributes attr = {0};
        Window window = windows[i];

        int attr_status = XGetWindowAttributes(ctx->disp, window, &attr);
        
        if(attr_status == 0) {
            continue;
        }

        if(!x11_is_window_fullscreen(ctx, window)) {
            continue;
        }
        if(attr.map_state != IsViewable) {
            continue;
        } 

        if(!attr.override_redirect) {
            continue;
        }

        if(!attr.map_installed) {
            continue;
        }

        XFree(windows);
        return true;
    }

    XFree(windows);

    return false;
}

bool x11_is_screen_off(X11Context *ctx) {
    CARD16 level;
    BOOL enabled;

    if(!DPMSInfo(ctx->disp, &level, &enabled)) {
        return false;
    }

    return level != DPMSModeOn;
}
