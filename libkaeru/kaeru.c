#include <stdio.h>

#include "kaeru.h"

image_type_t probe_image(FILE *fp) {
    return GIF;
}

extern void process_image(FILE *fp, image_t *img, image_type_t img_type) {
	switch (img_type) {
	case GIF:
		gif_process_image(fp, img);
		break;
	default:
		printf("Unsupported filetype.\n");
		break;
	}
}