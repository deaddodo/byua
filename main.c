#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "shared.h"

typedef struct {
    unsigned short w;
    unsigned short h;
    unsigned char  meta;
    unsigned char  bg_color_idx;
    unsigned char  px_aspect_ratio;
} __attribute__((packed)) logical_screen_descriptor_t;

typedef struct {
    unsigned char r, g, b, a;
} rgba_t;

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

void print_block(rgba_t c) {
    printf("\x1b[48;2;%u;%u;%um \x1b[0m", c.r, c.g, c.b);
}

logical_screen_descriptor_t read_header(FILE **fp) {
    unsigned char check[3];
    logical_screen_descriptor_t lsd = {0,0,0,0,0};
    bool bad_header = false;
    
    fread(&check, 1, 3, *fp);
    if(!(check[0] == 'G' &&
         check[1] == 'I' &&
         check[2] == 'F')) {
        bad_header = true;
    }
        
    fread(&check, 1, 3, *fp);
    if(!bad_header &&
       !(check[0] == '8' &&
        (check[1] == '7' || check[1] == '9') &&
         check[2] == 'a')) {
        bad_header = true;
    }
        
    if(!bad_header) {
        fread(&lsd, 1, sizeof(logical_screen_descriptor_t), *fp);
    }
    
    return lsd;
}

void read_color_table(FILE **fp, color_table_t *ct) {
    (*ct).table = malloc((*ct).size * sizeof(rgba_t));
    
    printf("Color table: [");
    
    for(int i = 0; i < (*ct).size; i++) {
        fread(&(*ct).table[i], 1, 3, *fp);
        print_block((*ct).table[i]);
    }
    
    printf("]\n");
}

void close_color_table(color_table_t *ct) {
    free((*ct).table);
}

bool process_image_sblock(FILE **fp, color_table_t *gct, rgba_t *canvas, graphic_control_extension_t *gce) {
    unsigned char size;
    fread(&size, 1, 1, *fp);
    
    if(size != 0x00) {
        printf(" > Processing sub-block!\n");
        for(int i = 0; i < size; i++) {
            unsigned char code;
            fread(&code, 1, 1, *fp);
        }
        return true;
    }
    
    return false;
}

void process_image_block(FILE **fp, color_table_t *gct, rgba_t *canvas, graphic_control_extension_t *gce) {
    image_descriptor_t id;
    color_table_t lct;
    color_table_t *ct;
    unsigned char code_size;
    bool data_left = true;
    
    fread(&id, 1, sizeof(image_descriptor_t), *fp);
    
    if(check_nth_bit(id.meta, 0)) {
        lct.active = true;
        lct.size = ((0x07&id.meta)+1);
        lct.size = lct.size * lct.size;
        read_color_table(fp, &lct);
        ct = &lct;
    } else {
        lct.active = false;
        ct = gct;
    }
    
    fread(&code_size, 1, 1, *fp);
    
    printf("Processing Image block...\n");
    while(data_left) {
        data_left = process_image_sblock(fp, ct, canvas, gce);
    }
    printf("Block process complete.\n");
    
    if(lct.active) {
        close_color_table(&lct);
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;
    logical_screen_descriptor_t lsd;
    color_table_t gct;
    graphic_control_extension_t gce = {0,0,0,0, false};
    rgba_t *canvas;
    bool data_left = true;
    
    fp = fopen(argv[1], "rb");
    
    lsd = read_header(&fp);
    canvas = malloc(lsd.w * lsd.h * sizeof(rgba_t));
    
    if(check_nth_bit(lsd.meta, 0)) {
        gct.active = true;
        gct.size = ((0x07&lsd.meta)+1);
        gct.size = gct.size * gct.size;
        read_color_table(&fp, &gct);
    } else {
        gct.active = false;
    }
    
    while(data_left) {
        unsigned char bt;
        fread(&bt, 1, 1, fp);
        
        switch(bt) {
        case 0x21: // extension
            fread(&bt, 1, 1, fp);
            if(bt == 0xF9) {
                fread(&gce, 1, sizeof(graphic_control_extension_t), fp);
                gce.set = true;
            }
            break;
        case 0x2c: // image
            process_image_block(&fp, &gct, canvas, &gce);
            gce.set = false; // no need to overwrite, just ignore
            break;
        case 0x3b: // trailer
            data_left = false;
            break;
        };
    }
    
    if(gct.active) {
        close_color_table(&gct);
    }
    
    free(canvas);
    fclose(fp);
    
    return 0;
}