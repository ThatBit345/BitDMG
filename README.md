# BitDMG
Simple gameboy emulator written in C++ with SDL3.

# Build
## Windows
Requirements:
- CMake 3.5 or greater
- Ninja
- gcc

1. `git clone --recurse-submodules https://github.com/ThatBit345/GameBoy-Emulator.git BitDMG`
2. `cd BitDMG`
3. `mkdir build`
4. `cmake -S . -B ./build -G Ninja -DCMAKE_BUILD_TYPE=Release`
5. `cmake --build ./build -j6`
