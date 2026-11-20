#include "x11.h"

void x11_init(X11Context *ctx) {
    Display* disp = XOpenDisplay(NULL);

    if(disp == NULL) {
        fprintf(stderr, "error: Unable to connect to X11\n");
        exit(1);
    }

    int opcode;
    int event;
    int error;

    if(!XQueryExtension(disp, "XInputExtension", &opcode, &event, &error)) {
        fprintf(stderr, "error: Failed to query X input extension. Error code: %d\n", error);
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
