/*-
 * Copyright (c) 2023-2025 Ruslan Bukin <br@bsdpad.com>
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

#ifndef _IMAGE_H_
#define _IMAGE_H_

#define	MAX_WIDTH		250
#define	MAX_HEIGHT		1920
#define	SERVER_SOCK_FILE	"/tmp/stage.sock"

struct ws_image {
	size_t width;
	size_t height;
	size_t size_in_bytes;
	size_t stride;
	int shmid;
	void *pixman;
};

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
	struct ws_output *output;
	struct wl_registry *wl_registry;
	struct wl_shm *wl_shm;
	struct zwlr_layer_shell_v1 *wlr_layer_shell;
	char *name;
	int margin_top;
	int margin_right;
	int margin_bottom;
	int margin_left;
	int width;
	int height;
	int anchor;
};

struct ws_image *ws_image_create(char *name, size_t width, size_t height);
void ws_image_destroy(struct ws_image *image);
void ws_image_draw(struct ws_image *image, pixman_color_t *color, char c,
    int offset_x, int offset_y);
void ws_font_init(void);
void ws_image_clear(struct ws_image *image, pixman_color_t *color, int x,
    int y, int w, int h);

#endif /* !_IMAGE_H_ */
