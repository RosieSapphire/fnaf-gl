# Five Nights at Freddy's GL
This is a project where I am recreating the entirety of the original Five Nights at Freddy's in OpenGL 3.3 & GLFW. This is open-source, and
will compile for Windows and Linux (I can't help with MacOS. lol). The big feature of this over the original FNaF is cross-compatibility.
Five Nights at Freddy's does not natively run on Linux due to it being restricted by its engine (Clickteam Fusion 2.5), and suffers compatibility
issues when trying to run though Wine or even Proton (at least from my personal experience).

## Will Run On
* Windows (tested)
* Linux   (tested)
* MacOS   (uhhhhh)

# Build
NOTICE: Since I am legally not allowed to distribute the original assets from Five Nights at Freddy's, they are not included in this repository.
For the time being, I am working with somebody on a custom decompiler that will extract the assets and put them all into their proper places
in the resource folder. This is a temporary problem and will be fixed as the project becomes closer to a playable state.
## Linux
### Build Dependencies
* Build Essential
* CMake
* Make
* Git
* Clang
* CGLM (needs to be compiled, see below)
* OpenAL
* GLFW
* SNDFile
* Alut
* FreeType

These examples are for debian-based distributions, but it'll work on anything as long as you know the correct package for your specific package manager.
```bash
sudo apt-get install clang build-essential make git libopenal-dev libglfw3-dev libsndfile-dev libalut-dev libfreetype-dev
```

First we need to compile CGLM from source, since it doesn't seem to work with the apt package version.
```bash
git clone https://github.com/recp/cglm
cd cglm
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCGLM_USE_C99=ON
sudo make install
```

Once we're finished running the "make install" command, you can delete the root cglm folder.
```bash
cd ../../
rm -rf cglm
```

Now we can clone this repository and enter the directory.
```bash
git clone https://github.com/RosieSapphire/fnaf-gl.git
cd fnaf-gl
```

Once inside, just type:
```bash
make release
```

Then you can run the game by doing
```bash
./five_nights_at_freddys
```
and you're off to the races! Enjoy.

### Windows
Honestly, I don't know other than creating a Visual Studio project out of it. I might update this repo to use CMake as to provide better
compatibility with Windows, but I may also just make it a separate repo or a fork. idk yet lol
