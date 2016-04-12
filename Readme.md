byua Image Viewer
====================

byua is a simple image viewer designed to be as embeddable as possible.

Components of byua:
 - *byua*     - Viewer
 - *libkaeru* - Decoder
 - *libmeka*  - Encoder

External dependencies:
 - [SDL2](http://libsdl.org) - required by byua for rendering.

## In Progress
Currently, only GIF decoding is supported. The following formats are the ideal targets (encode and decode):
 - [X] GIF
 - [ ] PNG
 - [ ] JPEG
 - [ ] BMP/DIB
 - [ ] TIFF
 - [ ] WEBP
 
 **Current functionality:**
 ![demo img](http://i.imgur.com/u9bnWRG.png)

## Data stream
|Stream|
|------|
|*kaeru* decodes to rgba array|
| **IN** |
|**_byau_ displays / works with rgba array**|
| **OUT** |
|*meka* encodes rgba array into output format|

*kaeru*/*meka* are stand alone and can be embedded elsewhere. *byua* controls the stream through the canvas (rgba_t array), while active.