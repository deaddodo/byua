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

|Format|Encode|Decode|
|------|------|------|
|GIF|:x:|:+1:|
| - Animation|:x:|:x:|
|PNG|:x:|:x:|
| - Animation|:x:|:x:|
|JPEG|:x:|:x:|
|BMP/DIB|:x:|:x:|
|TIFF|:x:|:x:|
|WEBP|:x:|:x:|
 
**Terminal demo:**

![demo img](http://i.imgur.com/u9bnWRG.png)

## Goals
### Data stream
|Stream|
|------|
|*kaeru* decodes to rgba array|
| **IN** |
|**_byau_ displays / works with rgba array**|
| **OUT** |
|*meka* encodes rgba array into output format|

*kaeru*/*meka* are stand alone and can be embedded elsewhere. *byua* controls the stream through the canvas (rgba_t array), while active.
