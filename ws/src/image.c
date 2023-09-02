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

#include <sys/mman.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <pixman.h>
#include <tllist.h>
#include <fcft/fcft.h>

#include "image.h"

static const char *font_list = "ubuntu mono:size=120,xos4 Terminus:size=120";
static struct fcft_font *font = NULL;
static enum fcft_subpixel subpixel_mode = FCFT_SUBPIXEL_DEFAULT;

struct ws_image *
ws_image_create(size_t width, size_t height)
{
	char shm_name[NAME_MAX];
	struct ws_image *image;
	pixman_image_t *pixman;
	void *buffer;
	size_t size;
	int shmid;
	int i;

	shmid = -1;
	size = width * height * 8;

	for (i = 0; i < UCHAR_MAX; ++i) {
		if (snprintf(shm_name, NAME_MAX, "/ws-%d", i) >= NAME_MAX)
			break;

		shmid = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (shmid > 0 || errno != EEXIST)
			break;
	}

	if (shmid < 0) {
		printf("shm_open() failed: %s", strerror(errno));
		return (NULL);
	}

	if (shm_unlink(shm_name) != 0) {
		printf("shm_unlink() failed: %s", strerror(errno));
		return (NULL);
	}

	if (ftruncate(shmid, size) != 0) {
		printf("ftruncate() failed: %s", strerror(errno));
		return (NULL);
	}

	buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
	if (buffer == MAP_FAILED) {
		printf("mmap() failed: %s", strerror(errno));
		return (NULL);
	}

	pixman = pixman_image_create_bits_no_clear(PIXMAN_a8r8g8b8, width,
	    height, buffer, width * 4);

	image = malloc(sizeof(struct ws_image));
	image->shmid = shmid;
	image->width = width;
	image->height = height;
	image->size_in_bytes = width * height * 4;
	image->stride = width * 4;
	image->pixman = pixman;

	return image;
}

void
ws_image_destroy(struct ws_image *image)
{
	pixman_image_t *pixman;

	pixman = image->pixman;
	pixman_image_unref(pixman);

	free(image);
}

void
ws_font_init(void)
{
	char **names;
	char *copy;
	char *name;
	size_t i;

	/* Instantiate font, and fallbacks. */
	tll(const char *)font_names = tll_init();

	copy = strdup(font_list);
	for (name = strtok(copy, ",");
	    name != NULL;
	    name = strtok(NULL, ",")) {
		while (isspace(*name))
			name++;

		size_t len = strlen(name);
		while (len > 0 && isspace(name[len - 1]))
			name[--len] = '\0';
		tll_push_back(font_names, name);
	}

	i = 0;
	names = malloc(sizeof (char *) * tll_length(font_names));
	tll_foreach(font_names, it)
		names[i++] = (char *)it->item;

	font = fcft_from_name(tll_length(font_names), (const char **)names,
	    NULL);
	assert(font != NULL);
	fcft_set_emoji_presentation(font, FCFT_EMOJI_PRESENTATION_DEFAULT);
	tll_free(font_names);
	free(copy);
}

void
ws_image_clear(struct ws_image *image, pixman_color_t *color)
{
	pixman_image_t *pixman;

	pixman = image->pixman;

	pixman_image_fill_rectangles(PIXMAN_OP_SRC, pixman, color, 1,
	    &(pixman_rectangle16_t){0, 0, MAX_WIDTH, MAX_HEIGHT});
}

void
ws_image_draw(struct ws_image *image, pixman_color_t *color,
    int ws, int offset_x, int offset_y)
{
	pixman_image_t *pixman;
	const struct fcft_glyph *g;
	pixman_image_t *clr_pix;
	char c;

	c = '0' + ws;

	g = fcft_rasterize_char_utf32(font, c, subpixel_mode);
	if (g == NULL)
		return;

	pixman = image->pixman;

	if (pixman_image_get_format(g->pix) == PIXMAN_a8r8g8b8) {
		pixman_image_composite32(
			PIXMAN_OP_OVER, g->pix, NULL, pixman, 0, 0, 0, 0,
			offset_x + g->x, offset_y + font->ascent - g->y,
			    g->width, g->height);
	} else {
		clr_pix = pixman_image_create_solid_fill(color);
		pixman_image_composite32(
			PIXMAN_OP_OVER, clr_pix, g->pix, pixman, 0, 0, 0, 0,
			offset_x + g->x, offset_y + font->ascent - g->y,
			    g->width, g->height);
	}
}
