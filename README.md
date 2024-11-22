# CH32V003 environmental sensor

## Development

Choose your toolchain for Windows (in order of suitability):
- [official MounRiver IDE](http://mounriver.com/download)
    - includes correct prebuilt libraries (rv32ec-ilp32e)
    - install via setup or extract it with innoextract, then look for the toolchain folder
    - includes two GCC versions: 8 and 12
    - these toolchains should also include support for a custom ISR attribute which is needed when using the HPE peripheral (?)
- [riscv-none-elf-gcc-xpack](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack)
    - includes rv32e-ilp32e libraries, which is good but not optimal as it doesn't use the compressed ISA
- manual [RISC-V GNU toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) build
    - slow and sizeable build, but worth it
- MSYS2 UCRT64 `riscv64-unknown-elf-toolchain`
    - does not include 32bit prebuilt libraries, only usable without standard libraries
- [SysProgs SysGCC 10](https://gnutoolchains.com/risc-v/)
    - only has rv32i prebuilt libraries, only usable without standard libraries

Choose your programming and debugging software (for the WCH-LinkE hardware tool):
- [official patched OpenOCD](https://github.com/openwch/openocd_wch)
    - find it in the MounRiver package
    - requires appropriate driver, found in MounRiver package at LinkDrv/WCHLink
- [minichlink](https://github.com/cnlohr/ch32v003fun/tree/master/minichlink)
    - currently broken debugging
        - `fix_gdb` branch should fix most issues: https://github.com/cnlohr/ch32v003fun/tree/fix_gdb/minichlink
    - has a custom debug terminal

You will also need to source a GNU Make build for Windows, possibly from MSYS2.

Use MSYS2 UCRT64 or ensure your terminal has make.exe in its PATH.

1. Update any relevant path in `Makefile`, `.vscode/c_cpp_properties.json` and `.vscode/launch.json`
2. Build with `make`
3. Set up programmer and connect 3V3, GND, SWDIO
4. Flash with `make flash`

For debugging with OpenOCD:
1. TODO

For debugging with minichlink:
1. Install `pacboy -Syu gdb-multiarch`
2. Download minichlink EXE and libusb DLL from https://github.com/cnlohr/ch32v003fun/tree/master/minichlink into the project root
3. Install WinUSB driver with Zadig on `WCH-Link (Interface 0)` for WCH-LinkE programmer

### Build toolchain in MSYS2 UCRT64

- https://github.com/riscv-collab/riscv-gnu-toolchain
- https://github.com/cnlohr/ch32v003fun/tree/master/misc
- https://github.com/cnlohr/ch32v003fun/issues/74

```sh
pacboy -Syu toolchain autoconf: automake: curl python3 mpc mpfr gmp gawk: base-devel: bison: flex: texinfo gperf libtool patchutils: bc zlib expat libslirp
git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain/
./configure --prefix $(pwd)/build-ch32v003 --with-arch=rv32ec --with-abi=ilp32e
make -j12 >make.txt 2>&1
```
The `:` after a package name in `pacboy` invocation is needed for packages which are only available for the base MSYS environment.  
Adjust the number of jobs (`-j12`) based on your CPU core count; the output redirection is there to avoid any slowdown caused by console output.

## TODO

- minichlink debugging is currently broken, try openocd_wch

## References

Based on template from: https://github.com/wagiminator/MCU-Templates/tree/main/CH32V003  

- Official HAL: https://github.com/openwch/ch32v003/tree/main/EVT/EXAM/SRC
- https://github.com/cnlohr/ch32v003fun
- https://github.com/basilhussain/ch32v003-startup
