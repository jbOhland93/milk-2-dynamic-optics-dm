# milk-2-Dynamic-Optics-DM

Program for interfacing milk with a Dynamic Optics deformable mirror. Waits on an image stream to update and immediately writes its values to the channels of the deformable mirror.

cacao-repository:
https://github.com/cacao-org/cacao

milk-repository:
https://github.com/milk-org/milk

## Build and Execution Instructions:
**Requirements:**
- Linux OS - tested on Ubuntu 20.04.6 LTS
- g++ installed - c++ compiler
- <Preliminary: Installed DO API>

| Terminal Command | Description |
| ------ | ------ |
| `make` | Builds the software. |
| `make clean` | Deletes the build directory. |
| `make run SETTINGS_FILE=<dm-settings-file>` | Builds and executes the software after it has already been built. Shortcut for `./build/milk-2-dynamic-optics-dm <dm-settings-file>` |

Close the program with Ctrl+C - the cleanup will be done automatically.
