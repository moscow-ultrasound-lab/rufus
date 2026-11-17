# Build Instructions for RUFUS

## Requirements

- Visual Studio Community 2019 (version 16.8.3 or later)
- Qt 5.15.2 (msvc2015_64 for x64, msvc2019 for x86)
- Qwt 6.1.5
- XRAD library 


## XRAD Library

RUFUS depends on the XRAD library. You can obtain it from:

- Repository: https://github.com/Center-of-Diagnostics-and-Telemedicine/xrad


Place the XRAD source code in the repository's `XRAD` directory. The project falls back to this local path automatically.

Required XRAD modules:
- XRADBasic
- XRADSystem
- XRADGUI

## matio

Install the x64 matio package with the bundled vcpkg installation:

```cmd
C:\dev\vcpkg\vcpkg.exe install matio:x64-windows
```



## HighFive Library

cmd

cd C:\dev
bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
bash
.\vcpkg install highfive:x64-windows
bash
.\vcpkg integrate install
Add to environment variable PATH value C:\dev\HighFive\include\highfive


## Environment Variables

Set the following system environment variables:
QTDIR5x64=C:\Qt\5.15.2\msvc2019_64
QTDIR5x86=C:\Qt\5.15.2\msvc2019
QtMsBuild=C:\Users\\[YourUsername]\AppData\Local\QtMsBuild
QWTDIR5x64=C:\QWT\6.1.5\VS2019\Qt5.15.2-x64
QWTDIR5x86=C:\QWT\6.1.5\VS2019\Qt5.15.2-x86
XRAD_LibDir=Q:\XRADLibrariesBuild\lib\x64-Release-2019
XRADRoot=Q:\XRAD



## Preparing the Source Code

1. Extract Qt and Qwt archives to `C:\` (as per the paths above)
2. Extract QtMsBuild to `C:\Users\[YourUsername]\AppData\Local\`
3. Place XRAD source code in the repository's `XRAD` directory. The project uses this local directory automatically; an external `Q:` drive is no longer required.


## Building with CMake

# Requirements

 - CMake (version 3.23 or later): https://cmake.org/download/

# Steps

 - Execute Q.bat
 - Open repository folder in terminal (right mouse button in explorer with repository folder opened -> "Open in terminal")
 - Execute following commands:
 -- cd projects/rufus-main
 -- cmake -B build --fresh
 -- cmake --build build --clean-first
 -- cd build/Debug
 -- ./RUFUS.exe

## Legacy building

1. Open `RUFUS.sln` in Visual Studio
2. Select `Release x64` configuration
3. Build the solution
After a successful build, all executables will be in `RUFUS_bin\`. Run `rufus-main.exe`.