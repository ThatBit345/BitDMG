# BitDMG
Simple gameboy emulator written in C++ with SDL3.

# Features
- Mapper support: MBC1, MBC2
- Window scaling

# Controls
- DPad: Arrow keys
- A: X
- B: Z
- START: RETURN
- SELECT: BACKSPACE
- 1->4: Change palette

# Build
## Windows
Requirements:
- CMake 3.5 or greater
- Ninja
- gcc

1. `git clone --recurse-submodules https://github.com/ThatBit345/BitDMG.git BitDMG`
2. `cd BitDMG`
3. `mkdir build`
4. `cmake -S . -B ./build -G Ninja -DCMAKE_BUILD_TYPE=Release`
5. `cmake --build ./build -j6`

# Usage
Drop a rom file on `BitDMG.exe` or, using a terminal, write the path to the rom as the first argument.
