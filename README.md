# CoD4x Server
A custom CoD4 Dedicated Server mainly to improve the functionality and usability of the stock CoD4 Server.

## License
This software is released under the GNU General Public License v3.0

## Compiling
To compile CoD4x-Server you'll need the following installed:
* gcc (Linux) or mingw32 (Windows)
* nasm

Debian/Ubuntu 32-bit:
```bash
sudo apt-get install nasm build-essential
```

Debian/Ubuntu 64-bit:
```bash
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install nasm:i386 build-essential gcc-multilib g++-multilib
```

openSUSE 32-bit:
```bash
sudo zypper install nasm gcc-32bit
```

Arch Linux 32-bit:
```bash
yaourt -S nasm gcc-multilib make
```

Compiling is as simple as running the appropriate build script. E.g. for Linux: `./build_elf.sh`.
