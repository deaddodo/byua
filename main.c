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
    
    for(int i = 0; i < (*ct).size; ++i) {
        fread(&(*ct).table[i], 1, 3, *fp);
        print_block((*ct).table[i]);
    }
    
    printf("]\n");
}

void close_color_table(color_table_t *ct) {
    free((*ct).table);
}

/************/
typedef struct {
  unsigned char byte;
  int prev;
  int len;
} dictionary_entry_t;

void decompress(int code_length,
               const unsigned char *input,
               int input_length,
               unsigned char *out) {
  int maxbits;
  int i, bit;
  int code, prev = -1;
  dictionary_entry_t *dictionary;
  int dictionary_ind;
  unsigned int mask = 0x01;
  int reset_code_length;
  int clear_code; // This varies depending on code_length
  int stop_code;  // one more than clear code
  int match_len;

  clear_code = 1 << ( code_length );
  stop_code = clear_code + 1;
  // To handle clear codes
  reset_code_length = code_length;

  // Create a dictionary large enough to hold "code_length" entries.
  // Once the dictionary overflows, code_length increases
  dictionary = ( dictionary_entry_t * ) 
    malloc( sizeof( dictionary_entry_t ) * ( 1 << ( code_length + 1 ) ) );

  // Initialize the first 2^code_len entries of the dictionary with their
  // indices.  The rest of the entries will be built up dynamically.

  // Technically, it shouldn't be necessary to initialize the
  // dictionary.  The spec says that the encoder "should output a
  // clear code as the first code in the image data stream".  It doesn't
  // say must, though...
  for ( dictionary_ind = 0; 
        dictionary_ind < ( 1 << code_length ); 
        dictionary_ind++ )
  {
    dictionary[ dictionary_ind ].byte = dictionary_ind;
    // XXX this only works because prev is a 32-bit int (> 12 bits)
    dictionary[ dictionary_ind ].prev = -1;
    dictionary[ dictionary_ind ].len = 1;
  }

  // 2^code_len + 1 is the special "end" code; don't give it an entry here
  dictionary_ind++;
  dictionary_ind++;
  
  // TODO verify that the very last byte is clear_code + 1
  while ( input_length )
  {
    code = 0x0;
    // Always read one more bit than the code length
    for ( i = 0; i < ( code_length + 1 ); i++ )
    {
      // This is different than in the file read example; that 
      // was a call to "next_bit"
      bit = ( *input & mask ) ? 1 : 0;
      mask <<= 1;

      if ( mask == 0x100 )
      {
        mask = 0x01;
        input++;
        input_length--;
      }

      code = code | ( bit << i );
    }

    if ( code == clear_code )
    {
      code_length = reset_code_length;
      dictionary = ( dictionary_entry_t * ) realloc( dictionary,
        sizeof( dictionary_entry_t ) * ( 1 << ( code_length + 1 ) ) );

      for ( dictionary_ind = 0; 
            dictionary_ind < ( 1 << code_length ); 
            dictionary_ind++ )
      {
        dictionary[ dictionary_ind ].byte = dictionary_ind;
        // XXX this only works because prev is a 32-bit int (> 12 bits)
        dictionary[ dictionary_ind ].prev = -1;
        dictionary[ dictionary_ind ].len = 1;
      }
      dictionary_ind++;
      dictionary_ind++;
      prev = -1;
      continue;
    }
    else if ( code == stop_code )
    {
      if ( input_length > 1 )
      {
        fprintf( stderr, "Malformed GIF (early stop code)\n" );
        exit( 0 );
      }
      break;
    }

    // Update the dictionary with this character plus the _entry_
    // (character or string) that came before it
    if ( ( prev > -1 ) && ( code_length < 12 ) )
    {
      if ( code > dictionary_ind )
      {
        fprintf( stderr, "code = %.02x, but dictionary_ind = %.02x\n",
          code, dictionary_ind );
        exit( 0 );
      }

      // Special handling for KwKwK
      if ( code == dictionary_ind )
      {
        int ptr = prev;

        while ( dictionary[ ptr ].prev != -1 )
        {
          ptr = dictionary[ ptr ].prev;
        }
        dictionary[ dictionary_ind ].byte = dictionary[ ptr ].byte;
      }
      else
      {
        int ptr = code;
        while ( dictionary[ ptr ].prev != -1 )
        {
          ptr = dictionary[ ptr ].prev;
        }
        dictionary[ dictionary_ind ].byte = dictionary[ ptr ].byte;
      }

      dictionary[ dictionary_ind ].prev = prev;

      dictionary[ dictionary_ind ].len = dictionary[ prev ].len + 1;

      dictionary_ind++;

      // GIF89a mandates that this stops at 12 bits
      if ( ( dictionary_ind == ( 1 << ( code_length + 1 ) ) ) &&
           ( code_length < 11 ) )
      {
        code_length++;

        dictionary = ( dictionary_entry_t * ) realloc( dictionary,
          sizeof( dictionary_entry_t ) * ( 1 << ( code_length + 1 ) ) );
      }
    }

    prev = code;

    // Now copy the dictionary entry backwards into "out"
    match_len = dictionary[ code ].len;
    while ( code != -1 )
    {
      out[ dictionary[ code ].len - 1 ] = dictionary[ code ].byte;
      if ( dictionary[ code ].prev == code )
      {
        fprintf( stderr, "Internal error; self-reference." );
        exit( 0 );
      }
      code = dictionary[ code ].prev;
    }

    out += match_len;
  }
}
/************/

void process_image_block(FILE **fp, color_table_t *gct, rgba_t *canvas, graphic_control_extension_t *gce) {
    image_descriptor_t id;
    color_table_t lct;
    color_table_t *ct;
    unsigned char code_size;
    unsigned char block_size;
    
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
    do {
        fread(&block_size, 1, 1, *fp);
    
        if(block_size != 0x00) {
            unsigned char *raw_data;
            unsigned char *processed_data;
            
            raw_data = malloc(sizeof(unsigned char)*block_size);
            for(int i = 0; i < block_size; ++i) {
                fread(&raw_data[i], 1, 1, *fp);
            }
            processed_data = malloc(id.w * id.h);
            decompress(code_size, raw_data, block_size, processed_data);
            for(int i = 0; i < (id.w * id.h); ++i) {
                canvas[i] = (*ct).table[processed_data[i]];
            }
            
            free(raw_data);
            free(processed_data);
        }
    } while(block_size != 0x00);
    printf("Block process complete.\n");
    
    if(lct.active) {
        close_color_table(&lct);
    }
}

void render(rgba_t *canvas, logical_screen_descriptor_t *lsd) {
    for(int i = 0; i < ((*lsd).w*(*lsd).h); ++i) {
        print_block(canvas[i]);
        if(!((i+1)%(*lsd).w))
            printf("\n");
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
    
    render(canvas, &lsd);
    
    free(canvas);
    fclose(fp);
    
    return 0;
}
