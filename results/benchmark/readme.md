# Benchmarking results

All the folders are structured in simple way. Every benchmark folder contains list of folders named 1000, 2000, etc. These are unlocking times in milliseconds passed to the benchmarking function as inputs. Every folder then contains set of csv files called stdmemN.csv, where N signifies number of threads used for Argon2. The file stats contains summarised statistics made with the Datamash tool. See script analyse_benchmarks.sh in scripts folders for more info.

## Laptop

These results were benchmarked on Lenovo Thinkpad P50 laptop with Intel(R) Core(TM) i7-6820HQ CPU @ 2.70GHz with 4 cores, 8 threads. The laptop was equipped with 32 GiB of SODIMM DDR4 Synchronous 2133 MHz (0,5 ns) RAM.

All benchmarks were done on 8 characters long password, except where stated othervise. See folder src in this repo for source code of benchmarking program.

There are following folders:

- cpu_count - benchmarked while disabling individual threads and then even whole cores no limitations of memory

- longer_passwords - just a test benchmark using 16 and 32 chars long password done on all cpus and no memory limitations

- memory_limits - simulated various memory limitations and requirements

- optimized - Cryptsetup was compiled with improved benchmark version, folders with "sse" in the name. The folders with name "external" were compiled agains external Argon2 library from the distro.

## Raspberry

The second configuration comprises Raspberry Pi 3, model B+. This device is equipped with quad core Broadcom CM2837B0, Cortex-A53 (ARMv8) 64-bit SoC @ 1.4GHz processor and 1GB LPDDR2 SDRAM memory.

As the memory is pretty liited already, I just performed benchmarks while disabling individual cores.