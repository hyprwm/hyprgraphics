# hyprgraphics

Hyprgraphics is a small C++ library with graphics / resource related utilities used across the hypr* ecosystem.

## Stability

Hyprgraphics depends on the ABI stability of the stdlib implementation of your compiler. Sover bumps will be done only for hyprgraphics ABI breaks, not stdlib.

## Building

```sh
git clone https://github.com/hyprwm/hyprgraphics
cd hyprgraphics/
cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr -S . -B ./build
cmake --build ./build --config Release --target all -j`nproc 2>/dev/null || getconf _NPROCESSORS_CONF`
sudo cmake --install build
```