=============
node-pykstbi
=============

Bindings for `stb_image.c`.

Installation
=============

In the working directory::

    npm install

API
====
::

    object pykstbi.loadFromMemory(Buffer buffer, int req_comp=0);

Loads an image from memory. Paletted PNG, BMP, GIF and PIC images 
are automatically depalettized.

Parameters:

buffer
    A buffer containing the image file data.

req_comp
    Number of components N in the decoded image data.

    An output image with N components has the following components
    interleaved in this order in each pixel:

    ======== ==========================
    N        Components
    ======== ==========================
    1        grey
    2        grey, alpha
    3        red, green, blue
    4        red, green, blue, alpha
    ======== ==========================

On success, returns the following object::

    {
        Buffer data,
        int width,
        int height,
        int comp
    }

where:

data
    A buffer consisting of image pixel data of height scanlines of
    width pixels, with each pixel consisting of N interleaved
    8-bit components, where N is req_comp if req_comp is non-zero,
    and comp otherwise. The first pixel in this buffer is the 
    top-left-most of the image. There is no padding between image
    scanlines or between pixels.

width
    The width of the image, in pixels.

height
    The height of the image, in pixels.

comp
    The number of components pixels in `data` will have if `req_comp`
    is 0.

On failure, returns null.

License
========
MIT License
