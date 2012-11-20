Motion Masks
============

Bitmap compositing driven by RLE-compressed alpha masks.

Keywords: RLE alpha animation bitmap blend buffer clip clipping compact composite compressed framebuffer mask motion opacity stencil transition transparency wipe

The Idea
--------

This is a system for compositing images by means of compressed alpha masks. The compression is lightweight enough to enhance memory bandwidth.

Source images, typically 8bpp greyscale, are read in and compressed into an efficient format which specifies whether to copy or blend the respective source images.

This improves matters because alpha masks are typically sparse in nature, with long runs of identical pixels.

Multiple masks can be packed together providing for animated transitions and removing redundancy.

Source Code Organisation
------------------------

* Platform independent code lives in `include` and `libraries`.
* `include` contains only public headers.
* `libraries` contains private headers and source code.
* Platform-specific code lives in `platform`.

Building the Test App
---------------------

Open `platform/macos/MotionMasks/MotionMasks.xcodeproj`.  
You'll need to adjust the file paths at the top of `PlotView.m` to point to some 320x480 images. It's hardcoded at the moment. Sorry about that. It's early days. It'll get fixed.  
Run that. It'll open up a window which composites as you move the mouse around.

Compression
-----------

The compression method used is RLE. This is simple enough to not require any heavy grunt work while plotting, but small enough to allow memory bandwidth requirements to be reduced.

### Method

(The following steps are is how compression is __intended__ to work. The present code only performs a subset of these operations).

1. Load a bunch of bitmaps.
2. Number each scanline.
3. Hash each scanline.
4. Remove duplicates.
5. Compress each unique scanline.
6. Write out offsets which point to unique scanlines.

Rendering
---------

This is just like a regular bitmap plot except that each row decodes a sequence of commands. The commands specify whether to plot, or blend between the corresponding two source image pixels and plot the result.

Two source images are available concurrently. The copy command can select either, the blend commands implicitly use both.

Other commands exist, such as one which selects the respective source images for copies and blends.

Bitmaps & Screens
-----------------

The Motion Mask code provides its own abstractions for bitmaps, screens and pixel formats, rather than operating on OS-specific types:

* `bitmap_t`
* `screen_t`

There is also a `bitmap_set_t` which is just like a `bitmap_t` but provides for multiple base pointers, allowing a series of images with identical dimensions and depth to be specified.

Pixel Formats
-------------

Pixel formats are specified with a `pixelfmt_t`. This enumeration has many pixels formats but only two are implemented so far: `rgbx8888` and `xbgr8888`.

Pixel Handlers
--------------

The copying and blending of specific pixel formats are coded in a `span_t`.

Notes
-----

* Written in straight C (no C99-isms, I think, save for the stdint types.)
* Portable across OSes.
* Tested on 32-bit and 64-bit [Note: presently there are hacks in there for 64-bit.]
* Intended to have critical pixel-blending portions coded in assembly for speed.
* Intended for embedded use where no GPU available.
* Cocoa test app seems a painful way to get bitmaps on-screen. It's very slow.
* Limits - two images per-pixel, sixteen source images which can be selected mid-stream.
* Streams - abstracted loading mechanism.
* It's easy to make upside-down bitmaps (just flip rowbytes and set the base pointer to the start of the last scanline.)
* 64K limit on data.

Current Status
--------------

* Compression is very basic, but functions.
* Rendering works.

There is an OS X Cocoa test app ("MotionMaskTest") in `platform/macos` which loads a PNG from a (presently hard coded) location. This is packed into a Motion Mask and written out to disc. It's then loaded back in and some JPEGs are loaded and displayed through the Motion Mask. The mouse can be moved around the window to draw at different offsets.

File Format
-----------

### Header
...

### Frames
...

### Offsets
...

### Data
...


Binary Encoding
---------------

The count of leading zeros of the initial byte is the unique identifier of the operation.

Command          | Binary format                     | Description
---------------- | --------------------------------- | -----------------------------------------
Copy             | `1sllllll`                        | Copy from source `s` up to 2^6 pixels.
Blend const      | `01llllll aaaaaaaa`               | Alpha blend up to 2^6 pixels, constant alpha.
Blend array      | `001lllll aaaaaaaa[len]`          | Alpha blend up to 2^5 pixels, variable alpha.
Long copy        | `0001slll llllllll`               | Copy from source `s` up to 2^11 pixels.
Long blend const | `00001lll llllllll aaaaaaaa`      | Alpha blend up to 2^11 pixels, constant alpha.
Long blend array | `000001ll llllllll aaaaaaaa[len]` | Alpha blend up to 2^10 pixels, variable alpha.
Undefined        | `0000001u`                        | Reserved for future use.
Set source       | `00000001 ttttssss`               | Set source images 0 and 1 to `s` and `t`. (Source zero is the screen, source N is input image N-1).
EOL              | `00000000`                        | End of line.

To Do
-----

* Lots and lots!
* Build environments for non-OS X platforms!
* Regression tests!
* Offsets ought to be RLE'd! (They make up the bulk of the size of the Motion Mask).
* Get some animated stuff going!
* Improve this documentation!
* Doxygenate the codebase!
* SetXY command! (To allow source image positions to be set).
* Porter-Duff!
* A library to support building custom Motion Masks from code!
* Ragged right hand edges (MotionMask don't need to be square).

History
-------

Author
------

David Thomas <dave@davespace.co.uk>

Copyright
---------

Copyright © David Thomas, 2012. All Rights Reserved. [Licensing will be sorted out once the damn thing works].
