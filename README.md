# hyprgraphics

Hyprgraphics is a small C++ library with graphics / resource related utilities used across the hypr* ecosystem.

## Stability

Hyprgraphics depends on the ABI stability of the stdlib implementation of your compiler. Sover bumps will be done only for hyprgraphics ABI breaks, not stdlib.

## Dependencies

Requires a compiler with C++26 support.

Dep list:
 - pixman-1
 - cairo
 - hyprutils
 - libjpeg
 - libwebp
 - libjxl
 - libjxl_cms
 - libjxl_threads
 - libmagic

## Building

```sh
git clone https://github.com/hyprwm/hyprgraphics
cd hyprgraphics/
cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr -S . -B ./build
cmake --build ./build --config Release --target all -j`nproc 2>/dev/null || getconf NPROCESSORS_CONF`
sudo cmake --install build
```
