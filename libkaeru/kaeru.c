#include <stdio.h>

#include "kaeru.h"

image_type_t kaeru_probe_image(FILE *fp) {
    return GIF;
}

extern void kaeru_process_image(FILE *fp, image_t *img, image_type_t img_type) {
	switch (img_type) {
	case GIF:
		gif_process_image(fp, img);
		break;
	default:
		printf("Unsupported filetype.\n");
		break;
	}
}