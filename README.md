# sdlretro
This is a lightweight libretro frontend written for SDL, aims to write simple codes that are easy to maintain, and allows to use libretro codes in SDL on the FunKey S.

# Build
* Use CMake 3.0 or later to build the project
* SDL 1.x and FreeType are requrired, and all platforms that supported by SDL 1.x(in window mode or fbdev IPU scaling) and FreeType should be supported as well
* for the FunKey S:
  1. create `build` folder and run `cmake -DCMAKE_BUILD_TYPE=Release -DMODEL=funkey ..` in it
  2. run `make` to build the binary
  3. run `./make_opk.sh` in folder `opk` to build Lite Version
  4. or create a folder `cores` in `opk` and put all core files into the folder, then run `FULL=1 ./make_opk.sh` to build Full Version

# Installation for FunKey
* Full version:
  Just copy sdlretro-full.opk to app folder
* Lite version:
  Copy sdlretro.opk to app folder, create /mnt/FunKey/.sdlretro/cores and put core files in the directory.

# Run
* Select game rom to run, if rom file type is supported more than one core, a core selection menu is popup first.
* Rom in zip is supported if there are no more than 2 files (one rom file and one dir/readme) in the zip. If the core does not support in-memory rom load, the rom in zip will be extracted to .sdlretro/tmp and removed on exit.
* Use power-flick to enter menu.

## CREDITS
* [libretro](https://github.com/libretro/libretro-common)
* [JSON for Modern C++](https://github.com/nlohmann/json)
* [miniz](https://github.com/richgel999/miniz)
* [stb](https://github.com/nothings/stb)
* [glad](https://github.com/Dav1dde/glad)
* [libsamplerate](https://github.com/erikd/libsamplerate)
* [xxHash](https://github.com/Cyan4973/xxHash)
* [cpuid](https://github.com/steinwurf/cpuid)
* [wingetopt](https://github.com/alex85k/wingetopt)
