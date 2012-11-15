Motion Masks
============

Compositing driven by RLE-compressed alpha masks.

Keywords: RLE alpha animation bitmap blend buffer clip clipping compact composite compressed framebuffer mask motion opacity stencil transition transparency wipe

The Idea
--------

This is a system for building and rendering RLE compressed masks/stencils which can be used to composite images.

The "Motion" part - multiple masks can be packed together providing for animated transitions.

Source Code Organisation
------------------------

* Platform independent code lives in `include` and `libraries`.
* `include` contains only public headers.
* Platform-specific lives in `platform`.

Building
--------

Open `platform/macos/MotionMasks/MotionMasks.xcodeproj`.  
You'll need to adjust the file paths at the top of `PlotView.m`. Sorry about that. It's early days.  
Run that.  

Compression
-----------

(This is how compression is intended to work. The present code is very basic).

1. Load a bunch of bitmaps.
2. Number each scanline.
3. Hash each scanline.
4. Remove duplicates.
5. Compress each unique scanline.
6. Write out offsets which point to unique scanlines.

Rendering
---------

...

Bitmaps & Screens
-----------------

The Motion Mask code provides its own abstractions for bitmaps, screens and pixel formats:

* `bitmap_t`
* `screen_t`

There is also a `bitmap_set_t` which is just like a `bitmap_t` but provides for multiple base pointers, allowing a series of images with identical dimensions and depth to be specified.

Pixel Formats
-------------

* `pixelfmt_t`

Implemented so far: rgbx8888 and xbgr8888.

Pixel Handlers
--------------

* `span_t`

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

Current Status
--------------

* Compression is very basic, but functions.
* Rendering works.

There is an OS X Cocoa test app ("MotionMaskTest") in `platform/macos` which loads a PNG from a (presently hard coded) location. This is packed into a Motion Mask and written out to disc. It's then loaded back in and some JPEGs are loaded and displayed through the Motion Mask. The mouse can be moved around the window to draw at different offsets.

Binary Encoding
---------------

The count of leading zeros of the initial byte is the unique identifier of the operation.

Command          | Binary format                     | Description
---------------- | --------------------------------- | -----------------------------------------
Copy             | `1sllllll`                        | Copy from source `s` up to 2^6 pixels.
Blend const      | `01llllll aaaaaaaa`               | Alpha blend in current style up to 2^6 pixels.
Blend array      | `001lllll aaaaaaaa[len]`          | Alpha blend in current style up to 2^5 pixels.
Long copy        | `0001slll llllllll`               | Copy from source `s` up to 2^11 pixels.
Long blend const | `00001lll llllllll aaaaaaaa`      | Alpha blend in current style up to 2^11 pixels.
Long blend array | `000001ll llllllll aaaaaaaa[len]` | Alpha blend in current style up to 2^10 pixels.
Undefined        | `0000001u`                        | Reserved for future use.
Set source       | `00000001 ttttssss`               | Set source images 0 and 1 to `s` and `t`.
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

History
-------

Author
------

David Thomas <dave@davespace.co.uk>

Copyright
---------

Copyright © David Thomas, 2012. All Rights Reserved. [Licensing will be sorted out once the damn thing works].
