#ifndef __KAERU_H__
#define __KAERU_H__
#include <stdio.h>
#include <stdbool.h>

#include "types.h"
#include "shared.h"
#include "gif.h"

extern image_type_t probe_image(FILE *fp);
extern void process_image(FILE *fp, image_t *img, image_type_t img_type);
#endif