# RUFUS — RF Ultrasound Framework for Universal Simulations

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.XXXXXXX.svg)](https://doi.org/10.5281/zenodo.XXXXXXX)

RUFUS is an open‑source C++ framework for processing real radio‑frequency (RF) data from ultrasound scanners. It implements key imaging modes — B‑mode, color flow, power Doppler, tissue Doppler, synthetic aperture focusing, aberration correction, and compression elastography — and is designed for testing and comparing novel signal processing algorithms.

## Features

- **B‑mode** – envelope detection, dynamic range compression, speckle reduction
- **Color flow imaging (CFM)** – clutter filtering, masking, velocity estimation
- **Power Doppler & tissue Doppler** – energy‑based and low‑velocity flow detection
- **Synthetic aperture imaging** – post‑processing focusing with aberration correction
- **Compression elastography** – strain estimation from multi‑frame B‑mode data
- **Real RF data support** – works with raw data from Sonomed‑500 scanners (extensible)
- **Open source** – MIT license, C++17, CMake build

## Repository structure
rufus/
├── projects/ # Main applications
│ ├── rufus-sa/ # Synthetic aperture + aberration correction
│ ├── rufus-doppler/ # Doppler modes (CFM, power, tissue)
│ ├── rufus-elasto/ # Compression elastography
│ └── CommonSources/ # Shared utilities
├── libs/
│ └── xrad-minimal/ # XRAD library (BSD‑3‑Clause)
├── LICENSE # MIT license
└── README.md


## Dependencies

- C++17 compiler
- CMake (≥ 3.15)
- Qt (core, widgets, svg)
- Qwt (for plots)

On Ubuntu:
```bash
sudo apt install libqt5svg5-dev libqwt-qt5-dev
On Windows (vcpkg):

bash
vcpkg install qt5 qwt
Building
bash
git clone https://github.com/moscow-ultrasound-lab/rufus.git
cd rufus
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="path/to/Qt;/path/to/Qwt"
make
License
RUFUS code (projects/): MIT

XRAD library (libs/xrad-minimal/): BSD‑3‑Clause

Citation
If you use RUFUS in your research, please cite:

Denis Leonov et al. RUFUS: RF Ultrasound Framework for Universal Simulations. DOI: XX.XXXX/zenodo.XXXXXX

Contact
Denis Leonov – strat89@mail.ru
Project page: github.com/moscow-ultrasound-lab/rufus



