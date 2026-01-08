# [UHF]RFID App for FlipperZero

<div style="
display: grid;
grid-template-columns: repeat(2, 1fr);
grid-template-rows: repeat(2, 1fr);
gap: 2px;
justify-items: center;
align-items: center;
width: 100vw;
">
  <img style="
    width: 280px;
    height: auto;"
    src="assets/img/uhf_demo_app2.jpg">
  <img style="
    width: 280px;
    height: auto;"
    src="assets/img/uhf_demo_app.jpg">
  <img style="
    width: 280px;
    height: auto;"
  src="assets/img/P_20231205_154700.jpg">
  <img style="
    width: 280px;
    height: auto;"
  src="assets/img/P_20231205_154717.jpg">
</div>

## Overview

This repository contains a UHF RFID application developed for FlipperZero, a versatile multi-tool device. The app leverages the YRM100 module to enable UHF RFID functionality.

## What's Ongoing

Adding features to the app to make it more usable and stable. Currently, also addressing the issue page to fix bugs and improve the app, as well as implementing planned and community-requested features.

## What's Changed (Stability & Crash Fixes)

This fork focuses on **stability, crash prevention, and compatibility with recent Flipper Zero firmware versions**.

### Fixed Crashes
- Fixed multiple crashes caused by invalid view and scene lifecycle handling.
- Prevented null-pointer dereferences when exiting scenes or disconnecting the UHF module.
- Fixed crashes when rapidly switching scenes or leaving the app during UART activity.
- Fixed memory leaks related to buffers, workers, and scene transitions.
- Prevented crashes when opening saved tags with incomplete or corrupted data.

### Stability Improvements
- Improved UART TX/RX handling to avoid race conditions.
- Added safer task and worker shutdown logic.
- Improved error handling when the UHF module does not respond.
- Improved application shutdown and cleanup flow.
- Reduced chances of UI lockups during long read/write operations.

### Firmware Compatibility
- Updated project structure to compile cleanly with modern Flipper Zero firmware.
- Fixed deprecated API usage warnings where possible.
- Ensured proper `.fap` packaging and build compatibility.

## Features

- [x] Read Single UHF RFID tag
  - [x] EPC Bank
  - [x] TID Bank
  - [x] USER Bank
- [x] View saved UHF RFID tags
- [x] Write Single UHF RFID tag
  - [x] EPC Bank
  - [ ] TID Bank (Not Supported if locked)
  - [x] USER Bank
- [ ] Change Module setting parameters **(In Progress)**
  - [x] Set Baudrates
  - [x] Set RF Power
  - [x] Set Region
  - [ ] Set/Reset Access Password
  - [ ] Set Kill Password
  - [ ] Kill Tag
- [ ] Edit/Create new data to write.
- Extras
  - [ ] Read multiple tags at once
  - [ ] View multiple on a list view

## Requirements

To run this application on FlipperZero, you will need:

- FlipperZero device (purchase from https://www.flipperdevices.com)
- YRM100 UHF RFID module (purchase from AliExpress)

## Setup and Installation

1. Ensure you have set up your FlipperZero device with the YRM100 module properly.
2. Clone this repository into the `applications_user/uhf_rfid` directory of your Flipper firmware.
3. Build the application using:
   ```
   ./fbt fap_uhf_rfid
   ```
4. Copy the generated `.fap` file to your Flipper Zero.

## Usage

1. Power on your FlipperZero device.
2. Connect the UHF module via GPIO.
3. Launch the UHF RFID app from the menu.
4. Read, save, and write EPC and USER banks.

## Contributions

Contributions are welcome. Please open a pull request or issue if you find bugs or want to improve the app.

## Disclaimer

- This application is provided as-is.
- Use at your own risk.
- The author is not responsible for any damage or data loss.

## Extra Resources

- MagicRF M100 & QM100 Firmware Manual
- TDS 1.9 Standard
- M5Stack UHF RFID Docs

## Contact

- Email: frux.infoc@gmail.com
- Discord Server: Flipper Zero Tutorial (Unofficial)
