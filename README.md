
# ApraPipes
A pipeline framework for developing video and image processing applications. Supports multiple GPUs and Machine Learning toolkits. More details can be found here https://apra-labs.github.io/ApraPipes.

## Build status
Automatically built and tested on Ubuntu 18.04, Jetson Boards and Windows 11 x64 Visual Studio 2017 Community (without CUDA)
|OS|Version|With Cuda|Tests|Status|
|--|-------|---------|------|------|
|Windows|2019|No|[![Test Results](https://gist.githubusercontent.com/kumaakh/f80af234a4aabedc69af3ee197f66944/raw/badge_Windows.svg)](https://gist.githubusercontent.com/kumaakh/f80af234a4aabedc69af3ee197f66944/raw/badge_Windows.svg)|[![CI-Win-NoCUDA](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Win-NoCUDA.yml/badge.svg)](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Win-NoCUDA.yml)|
|Windows|2019|Yes|[![Test Results](https://gist.githubusercontent.com/kumaakh/f80af234a4aabedc69af3ee197f66944/raw/badge_Windows-cuda.svg)](https://gist.githubusercontent.com/kumaakh/f80af234a4aabedc69af3ee197f66944/raw/badge_Windows-cuda.svg)|[![CI-Win-CUDA](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Win-CUDA.yml/badge.svg)](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Win-CUDA.yml)|
|Ubuntu x64_86|20.04|No|[![Test Results](https://gist.githubusercontent.com/kumaakh/f80af234a4aabedc69af3ee197f66944/raw/badge_Linux.svg)](https://gist.githubusercontent.com/kumaakh/f80af234a4aabedc69af3ee197f66944/raw/badge_Linux.svg)|[![CI-Linux-NoCUDA](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Linux-NoCUDA.yml/badge.svg)](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Linux-NoCUDA.yml)|
|Ubuntu ARM64 (Jetsons)|18.04|Yes|[![Test Results](https://gist.githubusercontent.com/kumaakh/f80af234a4aabedc69af3ee197f66944/raw/badge_Linux_ARM64.svg)](https://gist.githubusercontent.com/kumaakh/f80af234a4aabedc69af3ee197f66944/raw/badge_Linux_ARM64.svg)|[![CI-Linux-ARM64](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Linux-ARM64.yml/badge.svg)](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Linux-ARM64.yml)|
|Ubuntu x64_86-docker|18.04|Yes|No|[![CI-Linux-CUDA-Docker](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Linux-CUDA-Docker.yml/badge.svg)](https://github.com/Apra-Labs/ApraPipes/actions/workflows/CI-Linux-CUDA-Docker.yml)|



## Setup

### Prerequisites for CUDA 
* Make account on developer.nvidia.com, else the next steps will show HTTP 404/403 errors
* Download and install CUDA tool kit based on your OS: __Note: we test both with CUDA v10.2 and v11.7 so either is fine__
   * Ubuntu 18.04 : [CUDA Toolkit 10.2](https://developer.nvidia.com/cuda-10.2-download-archive?target_os=Linux&target_arch=x86_64&target_distro=Ubuntu&target_version=1804&target_type=debnetwork)
   * Windows 10/11 : [Cuda Toolkit 10.2](https://developer.nvidia.com/cuda-10.2-download-archive?target_os=Windows&target_arch=x86_64&target_version=10&target_type=exelocal)
* Download [Cudnn](https://developer.nvidia.com/rdp/cudnn-archive#a-collapse765-102) and extract where cuda is installed. Note: this is a painful process. Here are the steps:
   * Download the correct tar/zip file matching your cuda version. Do not download the exe/installer/deb package.
   * Windows: 
     * download [this file](https://developer.nvidia.com/compute/cudnn/secure/8.3.2/local_installers/10.2/cudnn-windows-x86_64-8.3.2.44_cuda10.2-archive.zip). 
     * Extract the downloaded file and copy files to ```C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.2``` using an __administrative command prompt__ as follows
       ```
       cd .\extracted_folder
       cd include
       copy *.h "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.2\include\"
       cd ..\lib
       copy *.lib "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.2\lib\x64\"
       cd ..\bin
       copy *.dll "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.2\bin\"
       ```
   * Linux:
      * download [this file](https://developer.nvidia.com/compute/cudnn/secure/8.3.2/local_installers/10.2/cudnn-linux-x86_64-8.3.2.44_cuda10.2-archive.tar.xz)
      * extract the files
        ``` 
        xz -d cudnn-linux-x86_64-8.3.2.44_cuda10.2-archive.tar.xz
        tar xvf cudnn-linux-x86_64-8.3.2.44_cuda10.2-archive.tar
        ```
      * copy files retaining the links
        ```
        cd ./cudnn-linux-x86_64-8.3.2.44_cuda10.2-archive
        sudo cp -P include/* /usr/local/cuda/include/
        sudo cp -P lib/* /usr/local/cuda/lib64/
        ```
## Windows
###  Prerequisites 
* Install Visual Studio 2019 Community 
  * Install Desktop development C++
  * .NET Desktop development
  * Universal Windows Development Platform
* Install choco:
  Open Windows PowerShell as Administrator and run:
  ```
  Set-ExecutionPolicy AllSigned
  Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
  ``` 
* Install build dependencies using choco: 
  ```
  choco feature enable -n allowEmptyChecksums && choco install 7zip git python3 cmake pkgconfiglite -y && pip3 install ninja && pip3 install meson
  ```
* Clone with submodules and LFS. 
  ```
  git clone --recursive https://github.com/Apra-Labs/ApraPipes.git
  ```
* Build libmp4
  ```
  cd thirdparty\libmp4
  .\build.cmd
  ```
* __Note__ As of this revision, there is no need to build thirdparty\gstreamer for windows as we leverage vcpkg for the same.

### Build for windows

#### Build Without Cuda
If your windows machies does not have an NVIDIA GPU use this script
```
build_windows_no_cuda.bat
```
#### Build With Cuda
```
build_windows_cuda.bat
```
### Run Tests
* list all tests
  ```
  _build/BUILD_TYPE/aprapipesut.exe --list_content
  ```
* run all tests  
  ```
  _build/BUILD_TYPE/aprapipesut.exe
  ```
* run all tests disabling memory leak dumps and better progress logging
  ```
  _build/BUILD_TYPE/aprapipesut.exe -p -l all --detect_memory_leaks=0
  ```
* run one test 
  ```
  _build/BUILD_TYPE/aprapipesut.exe --run_test=filenamestrategy_tests/boostdirectorystrategy
  ```
* run one test with arguments 
  ```
  _build/BUILD_TYPE/aprapipesut.exe --run_test=unit_tests/params_test -- -ip 10.102.10.121 -data ArgusCamera
  ```
  * Look at the unit_tests/params_test to check for sample usage of parameters in test code
 

## Ubuntu 18.04 and 20.04 x64
###  Prerequisites 
* Run the following to get latest build tools
  ```
  sudo apt-get update && sudo apt-get -y install   autoconf   automake  autopoint  build-essential  git-core  git-lfs libass-dev   libfreetype6-dev  libgnutls28-dev   libmp3lame-dev libsdl2-dev  libtool libsoup-gnome2.4-dev libncurses5-dev libva-dev   libvdpau-dev   libvorbis-dev   libxcb1-dev   libxcb-shm0-dev   libxcb-xfixes0-dev  ninja-build   pkg-config   texinfo   wget   yasm   zlib1g-dev   nasm   gperf bison curl zip unzip tar python3-pip flex && pip3 install meson
  ```  
* Note: start a new terminal as pip3 settings do not get effective on the same shell
* CMake minimum version 3.24 - Follow [this article](https://anglehit.com/how-to-install-the-latest-version-of-cmake-via-command-line/) to update cmake
* Clone with submodules and LFS. 
  ```
  git clone --recursive https://github.com/Apra-Labs/ApraPipes.git
  ```
* build gstreamer
   * ``` cd thirdparty && sh ./build_gstreamer.sh && cd -```
   * update .bashrc and append following line at the end of it. Adjust the path based on your environment.
     ```
     export LD_LIBRARY_PATH=~/ApraPipes/thirdparty/gst-build/gst-build-1.16/outInstall/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
     ```
   * load symbols from .bashrc ```source ~/.bashrc```. 
  
   
### Build for linux

* ```chmod +x build_linux_*.sh```
* ```./build_linux_x64.sh``` or ```./build_linux_no_cuda.sh``` depending on previous step. No Cuda as the name suggests will not build the Nvidia Cuda GPU Modules. Use this if there is no nvidia GPU present on your host

Build can take ~2 hours depending on the machine configuration.

### Build and test using docker
* Use this [docker image](https://github.com/users/kumaakh/packages/container/package/aprapipes-build-x86-ubutu18.04-cuda) with all the software setup.
  ```
  docker pull ghcr.io/kumaakh/aprapipes-build-x86-ubutu18.04-cuda:latest
  ```
* Run the docker container using above image
* Mount an external volume as a build area
* clone the repository with submodules and LFS as described above
* build using build_linux_\*.sh scripts as described [above](#build-for-linux)

This build will be fairly fast (~10 mins) as entire vcpkg cache comes down with the docker image

## Jetson boards - Nano, TX2, NX, AGX

### Prerequisites
* Setup the board with [Jetpack 4.4](https://docs.nvidia.com/sdk-manager/install-with-sdkm-jetson/index.html)
* run the following 
  ```
  sudo apt-get update && sudo apt-get -y install git-lfs libncurses5-dev ninja-build nasm curl libudev-dev && sudo snap install cmake --classic
  ```
* append following lines to ~/.bashrc
  ```
  export VCPKG_FORCE_SYSTEM_BINARIES=1
  export PATH=/usr/local/cuda/bin${PATH:+:${PATH}}
  export LD_LIBRARY_PATH=/usr/local/cuda/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
  ```
* reload ~/.bashrc:
  ```
  source ~/.bashrc:
  ```
* Clone with submodules and LFS. 
  ```
  git clone --recursive https://github.com/Apra-Labs/ApraPipes.git
  ```
* Run `./bootstrap-vcpkg.sh` in vcpkg/ directory
* Run `./vcpkg integrate install`

### Build for jetsons
* `chmod +x build_jetson.sh`
* `./build_jetson.sh`

Build can take ~12 hours on Jetson Nano. 
Note: Jetson build can also be done using Ubuntu 18.04 x86_64 Laptop via cross compilation. 

### Cross compilation using qemu
Conceptual steps adapted from [here](https://github.com/zhj-buffer/Cross-Compile-Jetson):

* On any Intel Ubuntu 18.04 computer (physical or virtual including wsl ) mount a Jetson SD Card Image as described above
* Copy relevant files from mounted image to created a rootfs 
* Install qemu on ubuntu host
* chroot into emulated aarm64 environment using script provided in the github link above
* install extra tools and build aprapipes and aprapipesut
* the built aprapipesut can be copied to a Jetson board and run. 

This approach can use all 12-16 cores of a laptop and hence builds faster.


### Run Tests
* list all tests `_build/aprapipesut --list_content`
* run all tests  `_build/aprapipesut`
* run one test `_build/aprapipesut --run_test=filenamestrategy_tests/boostdirectorystrategy`
* run one test with arguments `_build/aprapipesut --run_test=unit_tests/params_test -- -ip 10.102.10.121 -data ArgusCamera`
  * Look at the unit_tests/params_test to check for sample usage of parameters in test code

This project uses boost tests for unit tests.

## Update Submodules
```
git submodule update --init --recursive
```

## Update Documentation
If any changes are made in the documentation i.e. in /docs/source folder, the docs must be regenerated again follwing the steps given below. New contents from the /docs/build directory should be committed.

### To regenerate documentation
```
To build docs
apt-install get python-sphinx 
pip install sphinx-rtd-theme
cd docs
make html
```
