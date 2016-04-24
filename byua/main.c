#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "libkaeru/types.h"
#include "libkaeru/kaeru.h"

void print_block(rgba_t c) {
    printf("\x1b[48;2;%u;%u;%um \x1b[0m", c.r, c.g, c.b);
}

void render(image_t *img) {
    for(int i = 0; i < ((*img).width*(*img).height); ++i) {
        print_block((*img).canvas[0][i]);
        if(!((i+1)%(*img).width))
            printf("\n");
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;
    image_t img;
    
    fp = fopen(argv[1], "rb");
    
    kaeru_process_image(fp, &img, kaeru_probe_image(fp));
    
    render(&img);
    
    for(int i = 0; i < img.frames; ++i) {
        free(img.canvas[i]);
    }
    free(img.canvas);
    fclose(fp);
    
    return 0;
}
