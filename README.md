# PicoTTS

This is a copy of PicoTTS from android's source tree. It works perfectly on
Linux with effectively no change (I had to add a single `#include <stdint.h>`).

`build.sh` will build a static library `libpico.a` and a test application `test`.

`fetch.sh` will clone android's source tree and grab the latest version, so long
as it doesn't change location or get deleted. It getting deleted may be a real
concern, note the following commit message in the svox repo:

> Disable external/svox.
>
> Disable the build as a prelude to removing it from the manifest.
>
> This code isn't maintained and doesn't work when built 64-bit.

We don't store a copy of the svox repo here because it seems that a file in it
has been getting hit with (bogus?) DMCA notices [[1\]](https://github.com/DougGore/picopi/issues/5)
 [[2\]](https://github.com/ch3ll0v3k/picoPi2).