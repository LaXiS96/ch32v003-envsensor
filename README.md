# CH32V003 environmental sensor

## Development

Choose your toolchain for Windows (in order of suitability):
- [official MounRiver IDE](http://mounriver.com/download)
    - includes correct prebuilt libraries (rv32ec-ilp32e)
    - install via setup or extract it with innoextract, then look for the toolchain folder
    - includes two GCC versions: 8 and 12
    - these toolchains should also include support for a custom ISR attribute which is needed when using the HPE peripheral (?)
- manual [RISC-V GNU toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) build
    - slow and sizeable build, but worth it
- [riscv-none-elf-gcc-xpack](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack)
    - includes rv32e-ilp32e libraries, which is good but not optimal as it doesn't use the compressed ISA
- [SysProgs SysGCC 10](https://gnutoolchains.com/risc-v/)
    - only has rv32i prebuilt libraries, only usable without standard libraries
- MSYS2 UCRT64 `riscv64-unknown-elf-toolchain`
    - does not include 32bit prebuilt libraries, only usable without standard libraries

Choose your programmer and debugger (for the WCH-LinkE hardware tool):
- [official OpenOCD](https://github.com/openwch/openocd_wch)
    - find it in the MounRiver package
    - requires appropriate driver, found in MounRiver package at LinkDrv/WCHLink
    - flashing and debugging work as expected
- [minichlink](https://github.com/cnlohr/ch32v003fun/tree/master/minichlink)
    - debugging is a mixed bag, in VSCode it "works" but command line GDB is totally broken
    - has a custom debug terminal that requires a custom implementation MCU-side
    - flashing works OK

MSYS2 UCRT64 is recommended for development under Windows. Also note that using MSYS2 binaries outside an MSYS2 environment can cause problems, like missing DLL errors.

1. Update any relevant path in `Makefile`, `.vscode/c_cpp_properties.json` and `.vscode/launch.json`
2. Build with `make`
3. Set up programmer and connect 3V3, GND, SWDIO
4. Flash with `make flash`

Debugging with official OpenOCD (recommended):
1. Uninstall Zadig driver (if any): Device Manager > Universal Serial Bus devices > WCH-Link > Uninstall device
2. In its own terminal, run `make debug` or launch OpenOCD: `openocd.exe -f wch-riscv.cfg`
    - (`wch-riscv.cfg` may need a full path, it's in the same directory as `openocd.exe`)
3. Start debugging in VSCode with the correct launch configuration, or use `gdb-multiarch`/`riscv32-unknown-elf-gdb`:
```sh
$ riscv32-unknown-elf-gdb build/firmware.elf
(gdb) target extended-remote :3333 # Connect to OpenOCD
(gdb) monitor reset halt # Reset and halt MCU
(gdb) b main # Set up breakpoint on main function
(gdb) b *0x1b6 # Set up breakpoint on instruction at memory address
(gdb) i b # Display breakpoints
(gdb) d 2 # Delete breakpoint 2
(gdb) display/i $pc # Print the current instruction after stepping
(gdb) c # Continue execution
(gdb) si # Step one instruction
(gdb) x/16b $a2-1024 # Dump 16 bytes of memory starting at the address stored in register A2 minus 1024
(gdb) p $sp # Print value of SP register
```

Debugging with minichlink:
1. Install: `pacboy -Syu gdb-multiarch`
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

## References

Based on template from: https://github.com/wagiminator/MCU-Templates/tree/main/CH32V003  

- Official HAL: https://github.com/openwch/ch32v003/tree/main/EVT/EXAM/SRC
- https://github.com/cnlohr/ch32v003fun
- https://github.com/basilhussain/ch32v003-startup
