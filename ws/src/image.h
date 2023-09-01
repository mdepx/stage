#ifndef _IMAGE_H_
#define _IMAGE_H_

#define	MAX_WIDTH	200
#define	MAX_HEIGHT	1000

struct ws_image {
	size_t width;
	size_t height;
	size_t size_in_bytes;
	size_t stride;
	int shmid;
	void *pixman;
};

struct ws_image *ws_image_create(size_t width, size_t height);
void ws_image_destroy(struct ws_image *image);
void ws_image_draw(struct ws_image *image, pixman_color_t *color, int ws,
    int offset_x, int offset_y);
void ws_font_init(void);
void ws_image_clear(struct ws_image *image, pixman_color_t *color);

#endif /* !_IMAGE_H_ */
