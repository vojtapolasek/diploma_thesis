# Benchmarking results for contemporary laptop

These results were benchmarked on Lenovo Thinkpad P50 laptop with Intel(R) Core(TM) i7-6820HQ CPU @ 2.70GHz with 4 cores, 8 threads. The laptop was equipped with 32 GiB of SODIMM DDR4 Synchronous 2133 MHz (0,5 ns) RAM.

All benchmarks were done on 8 characters long password, except where stated othervise. See folder src in this repo for source code of benchmarking program.

There are following folders:

- cpu_count - benchmarked while disabling individual threads and then even whole cores no limitations of memory

- longer_passwords - just a test benchmark using 16 and 32 chars long password done on all cpus and no memory limitations

- memory_limits - simulated various memory limitations and requirements

- optimized - Cryptsetup was compiled with improved benchmark version, folders with "sse" in the name. The folders with name "external" were compiled agains external Argon2 library from the distro.

All the folders are structured in simple way. Every benchmark folder contains list of folders named 1000, 2000, etc. These are unlocking times in milliseconds passed to the benchmarking function as inputs. Every folder then contains set of csv files called stdmemN.csv, where N signifies number of threads used for Argon2. The file stats contains summarised statistics made with the Datamash tool. See script analyse_benchmarks.sh in scripts folders for more info.