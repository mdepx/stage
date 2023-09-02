/*-
 * Copyright (c) 2023 Ruslan Bukin <br@bsdpad.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pixman.h>

#include <fcft/fcft.h>

#include "image.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

static pixman_color_t fg = {0xffff, 0xffff, 0xffff, 0xffff};
static pixman_color_t bg = {0x0000, 0x0000, 0x0000, 0xffff};
static pixman_color_t mg = {0x4444, 0x4444, 0x4444, 0xffff};
static pixman_color_t og = {0x9999, 0x9999, 0x9999, 0xffff};

struct ws_surface {
	struct zwlr_layer_surface_v1 *wlr_layer_surface;
	struct wl_surface *wl_surface;
};

struct ws_output {
	struct wl_list link;
	struct wl_output *wl_output;
	struct ws_surface *ws_surface;
};

struct ws {
	struct ws_image *image;
	struct wl_buffer *wl_buffer;
	struct wl_compositor *wl_compositor;
	struct wl_display *wl_display;
	struct wl_list ws_outputs;
	struct wl_registry *wl_registry;
	struct wl_shm *wl_shm;
	struct zwlr_layer_shell_v1 *wlr_layer_shell;
};

void
layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *surface,
    uint32_t serial, uint32_t w, uint32_t h)
{

	zwlr_layer_surface_v1_ack_configure(surface, serial);
}

void
layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface)
{

}

const static struct zwlr_layer_surface_v1_listener
    zwlr_layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed,
};

struct ws_surface *
ws_surface_create(struct ws *app, struct wl_output *wl_output)
{
	struct ws_surface *ws_surface;
	int width, height;

	width = MAX_WIDTH;
	height = MAX_HEIGHT;

	ws_surface = calloc(1, sizeof(struct ws_surface));
	if (ws_surface == NULL) {
		printf("calloc failed");
		return (NULL);
	}

	ws_surface->wl_surface =
	    wl_compositor_create_surface(app->wl_compositor);
	if (ws_surface->wl_surface == NULL) {
		printf("wl_compositor_create_surface failed");
		return (NULL);
	}

	ws_surface->wlr_layer_surface =
	    zwlr_layer_shell_v1_get_layer_surface(app->wlr_layer_shell,
	    ws_surface->wl_surface, wl_output,
	    ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "ws");
	if (ws_surface->wlr_layer_surface == NULL) {
		printf("wlr_layer_shell_v1_get_layer_surface failed");
		return (NULL);
	}

#if 0
	zwlr_layer_surface_v1_set_margin(ws_surface->wlr_layer_surface,
	    margin_top, margin_right, margin_bottom, margin_left);
#endif

	zwlr_layer_surface_v1_set_size(ws_surface->wlr_layer_surface,
	    width, height);
	zwlr_layer_surface_v1_set_anchor(ws_surface->wlr_layer_surface, 0);
	zwlr_layer_surface_v1_add_listener(ws_surface->wlr_layer_surface,
	    &zwlr_layer_surface_listener, app);

	return (ws_surface);
}

void
ws_surface_destroy(struct ws_surface *ws_surface)
{

	zwlr_layer_surface_v1_destroy(ws_surface->wlr_layer_surface);
	wl_surface_destroy(ws_surface->wl_surface);

	free(ws_surface);
}

void
ws_output_destroy(struct ws_output *output)
{

	if (output->ws_surface != NULL)
		ws_surface_destroy(output->ws_surface);

	if (output->wl_output != NULL)
		wl_output_destroy(output->wl_output);

	free(output);
}

void
handle_global(void *data, struct wl_registry *registry, uint32_t name,
    const char *interface, uint32_t version)
{
	struct ws_output *output;
	struct ws *app;

	app = (struct ws *)data;

	if (strcmp(interface, wl_shm_interface.name) == 0)
		app->wl_shm = wl_registry_bind(registry, name,
		    &wl_shm_interface, 1);
	else if (strcmp(interface, wl_compositor_interface.name) == 0)
		app->wl_compositor = wl_registry_bind(registry, name,
		    &wl_compositor_interface, 1);
	else if (strcmp(interface, wl_output_interface.name) == 0) {

		if (version < 4) {
			printf("Unsupported version\n");
			return;
		}

		output = calloc(1, sizeof(struct ws_output));
		output->wl_output = wl_registry_bind(registry, name,
		    &wl_output_interface, version);

		wl_list_insert(&app->ws_outputs, &output->link);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0)
		app->wlr_layer_shell = wl_registry_bind(registry, name,
		    &zwlr_layer_shell_v1_interface, 1);
}

void
handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{

	/* No support. */
}

void
ws_flush(struct ws *app)
{
	struct ws_output *output;

	wl_list_for_each (output, &app->ws_outputs, link) {
		if (output->ws_surface != NULL) 
				continue;

		output->ws_surface = ws_surface_create(app,
		    output->wl_output);
		wl_surface_commit(output->ws_surface->wl_surface);
	}

	if (wl_display_roundtrip(app->wl_display) < 0)
		printf("wl_display_roundtrip failed");

	wl_list_for_each (output, &(app->ws_outputs), link) {
		if (output->ws_surface == NULL)
			continue;

		wl_surface_attach(output->ws_surface->wl_surface,
		    app->wl_buffer, 0, 0);
		wl_surface_damage(output->ws_surface->wl_surface, 0, 0,
		    INT32_MAX, INT32_MAX);
		wl_surface_commit(output->ws_surface->wl_surface);
	}

	if (wl_display_roundtrip(app->wl_display) < 1)
		printf("wl_display_roundtrip failed");
}

static void
draw_numbers(struct ws *app, char *buf)
{
	static pixman_color_t *color;
	int oldflag;
	int newflag;
	int voffs;
	int ws;
	int i;
	char c;

	i = 0;
	voffs = 100;
	oldflag = newflag = 0;

	ws_image_clear(app->image, &bg);

	while (1) {
		c = buf[i++];

		if (c == '\0')
			break;

		if (c == '?') {
			oldflag = 1;
			continue;
		}

		if (c == '!') {
			newflag = 1;
			continue;
		}

		ws = strtol(&c, NULL, 10);

		/* printf("c %c ws %d\n", c, ws); */

		if (newflag) {
			color = &fg;
			newflag = 0;
		} else if (oldflag) {
			color = &og;
			oldflag = 0;
		} else
			color = &mg;

		ws_image_draw(app->image, color, ws, 50, voffs);

		voffs += 120;
	}

	ws_flush(app);
}

int
ws_main_loop(struct ws *app)
{
	struct sockaddr_un addr;
	struct sockaddr_un from;
	socklen_t fromlen;
	char buf[128];
	int len;
	int fd;

	fromlen = sizeof(struct sockaddr_un);

	if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
		perror("socket");

	memset(buf, 0, 128);
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SERVER_SOCK_FILE);
	unlink(SERVER_SOCK_FILE);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror("bind");

	do {
		len = recvfrom(fd, buf, 8192, 0, (struct sockaddr *)&from,
		    &fromlen);
		/* printf("recvfrom: %s, len %d\n", buf, len); */
		draw_numbers(app, buf);
	} while (len > 0);

	close(fd);

	return (0);
}

static int
ws_startup(void)
{
	struct ws_output *output;
	struct wl_shm_pool *pool;
	struct ws_image *image;
	struct ws *app;

	app = calloc(1, sizeof(struct ws));
	image = ws_image_create(MAX_WIDTH, MAX_HEIGHT);

	app->image = image;

	wl_list_init(&app->ws_outputs);
	output = calloc(1, sizeof(struct ws_output));
	output->wl_output = NULL;
	wl_list_insert(&app->ws_outputs, &output->link);

	const static struct wl_registry_listener wl_registry_listener = {
		.global = handle_global,
		.global_remove = handle_global_remove,
	};

	app->wl_display = wl_display_connect(NULL);
	if (app->wl_display == NULL) {
		printf("wl_display_connect failed");
		return (-1);
	}

	app->wl_registry = wl_display_get_registry(app->wl_display);
	if (app->wl_registry == NULL) {
		printf("wl_display_get_registry failed");
		return (-1);
	}

	wl_registry_add_listener(app->wl_registry, &wl_registry_listener, app);

	if (wl_display_roundtrip(app->wl_display) < 0) {
		printf("wl_display_roundtrip failed");
		return (-1);
	}

	if (wl_display_roundtrip(app->wl_display) < 0) {
		printf("wl_display_roundtrip failed");
		return (-1);
	}

	if (app->wlr_layer_shell == NULL) {
		printf("No layer shell available\n");
		return (-1);
	}

	if (app->wl_shm == NULL) {
		printf("No wl_shm available\n");
		return (-1);
	}

	if (app->wl_compositor == NULL) {
		printf("No wl_compositor available\n");
		return (-1);
	}

	pool = wl_shm_create_pool(app->wl_shm, app->image->shmid,
	    app->image->size_in_bytes);
	if (pool == NULL) {
		printf("wl_shm_create_pool failed");
		return (-1);
	}

	app->wl_buffer = wl_shm_pool_create_buffer(pool, 0, app->image->width,
	    app->image->height, app->image->stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	if (app->wl_buffer == NULL) {
		printf("wl_shm_pool_create_buffer failed");
		return (-1);
	}

	ws_main_loop(app);

	zwlr_layer_shell_v1_destroy(app->wlr_layer_shell);
	wl_buffer_destroy(app->wl_buffer);
	wl_compositor_destroy(app->wl_compositor);
	wl_shm_destroy(app->wl_shm);
	wl_registry_destroy(app->wl_registry);
	wl_display_roundtrip(app->wl_display);
	wl_display_disconnect(app->wl_display);
	ws_image_destroy(app->image);
	free(app);

	return (0);
}

int
main(int argc, char **argv)
{

	fcft_init(FCFT_LOG_COLORIZE_AUTO, false, FCFT_LOG_CLASS_DEBUG);
	ws_font_init();
	ws_startup();

	return (0);
}
