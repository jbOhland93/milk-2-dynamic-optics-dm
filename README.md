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
- Dynamic Optics "PztMulti" API
- ImageStreamIO (shipped with milk)

| Terminal Command | Description |
| ------ | ------ |
| `make` | Builds the software. |
| `make clean` | Deletes the build directory. |
| `make run SETTINGS_FILE=<dm-settings-file>` | Builds and executes the software after it has already been built. Shortcut for `./build/milk-2-dynamic-optics-dm <dm-settings-file>` |

After building, export the build directory to the environmental variables:
``export MILK_TO_DO_DM_PATH=<path-to-repository>/build/``

The configuration for this C++ program, including the IP of the DM, the input ImageStreamIO stream name, and other parameters, is specified through a text file. This file's path must be provided as an argument when running the executable. For reference, an example file (``dm-settings-default``) containing all possible settings is located at the top level of this repository.
