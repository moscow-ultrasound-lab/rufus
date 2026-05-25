# Build Instructions for RUFUS

## Requirements

- Visual Studio Community 2019 (version 16.8.3 or later)
- Qt 5.15.2 (msvc2015_64 for x64, msvc2019 for x86)
- Qwt 6.1.5
- XRAD library 


## XRAD Library

RUFUS depends on the XRAD library. You can obtain it from:

- Repository: https://github.com/Center-of-Diagnostics-and-Telemedicine/xrad


Place the XRAD source code in `Q:\XRAD` (or set `XRADRoot` environment variable to the correct path).

Required XRAD modules:
- XRADBasic
- XRADSystem
- XRADGUI




## Environment Variables

Set the following system environment variables:
QTDIR5x64=C:\Qt\5.15.2\msvc2015_64
QTDIR5x86=C:\Qt\5.15.2\msvc2019
QtMsBuild=C:\Users[YourUsername]\AppData\Local\QtMsBuild
QWTDIR5x64=C:\QWT\6.1.5\VS2015\Qt5.15.2-x64
QWTDIR5x86=C:\QWT\6.1.5\VS2019\Qt5.15.2-x86
XRADRoot=Q:\XRAD



## Preparing the Source Code

1. Extract Qt and Qwt archives to `C:\` (as per the paths above)
2. Extract QtMsBuild to `C:\Users\[YourUsername]\AppData\Local\`
3. Create a virtual drive Q: pointing to the project root:
subst Q: D:\path\to\rufus


4. Place XRAD source code in `Q:\XRAD`

## Building

1. Open `RUFUS.sln` in Visual Studio
2. Select `Release x64` configuration
3. Build the solution

## Running

After a successful build, all executables will be in `RUFUS_bin\`. Run `rufus-main.exe`.