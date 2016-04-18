#ifndef __GIF_H__
#define __GIF_H__
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "shared.h"

typedef struct {
    unsigned short w;
    unsigned short h;
    unsigned char  meta;
    unsigned char  bg_color_idx;
    unsigned char  px_aspect_ratio;
} __attribute__((packed)) logical_screen_descriptor_t;

typedef struct {
    bool active;
    unsigned char size;
    rgba_t *table;
} color_table_t;

typedef struct {
    unsigned char size;
    unsigned char meta;
    unsigned short delay;
    unsigned char clear_idx;
    bool set;
} __attribute__((packed)) graphic_control_extension_t;

typedef struct {
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
    unsigned char meta;
} __attribute__((packed)) image_descriptor_t;

typedef struct {
	unsigned char byte;
	int prev;
	int len;
} dictionary_entry_t;

logical_screen_descriptor_t read_header(FILE **fp);
void read_color_table(FILE **fp, color_table_t *ct);
void close_color_table(color_table_t *ct);
void decompress(int code_length,
                const unsigned char *input,
                int input_length,
                unsigned char *out);
void process_image_block(FILE **fp,
                         color_table_t *gct,
						 rgba_t *canvas,
						 graphic_control_extension_t *gce);
void gif_process_image(FILE *fp, image_t *img);
#endif