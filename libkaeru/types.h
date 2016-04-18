#ifndef __TYPES_H__
#define __TYPES_H__
typedef enum image_type_t {
    GIF,
    BMP,
    JPG,
    TIF,
    PNG,
    WBP,
	INV
} image_type_t;

typedef struct {
    unsigned char r, g, b, a; // full 32-bit support
	unsigned int delay; // 0 if static
} rgba_t;

typedef struct {
	unsigned int width, height, frames;
	bool animate;
	rgba_t **canvas; // array (frames) of array (rgba canvas')
} image_t;
#endif