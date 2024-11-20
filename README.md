# CH32V003 environmental sensor

## Development

1. Use MSYS2 UCRT64 on Windows
2. Install toolchain: `pacboy -S riscv64-unknown-elf-toolchain gdb-multiarch`
3. Update any relevant path in `.vscode/c_cpp_properties.json` and `.vscode/launch.json`
4. Build with `make`
5. Download minichlink EXE and libusb DLL from https://github.com/cnlohr/ch32v003fun/tree/master/minichlink into the project root
6. Install WinUSB driver with Zadig on `WCH-Link (Interface 0)` for WCH-LinkE programmer
7. Connect 3V3, GND, SWDIO
8. Flash with `make flash`

## References

Based on template from: https://github.com/wagiminator/MCU-Templates/tree/main/CH32V003

- Official HAL: https://github.com/openwch/ch32v003/tree/main/EVT/EXAM/SRC
- https://github.com/cnlohr/ch32v003fun
- https://github.com/basilhussain/ch32v003-startup
