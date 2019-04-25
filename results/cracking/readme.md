# Cracking results

This folder presents results of simulating attack against Argon2 hashes with parameters benchmarked in previous step of the thesis.

## Cracker

This folder contains timing results of a custom made tool which uses naive method of guessing password of LUKS2 encrypted volume.

 Laptop and raspberry folder contain particular results, see the benchmarking folder or the thesis for parameters of devices.

Names of subfolders refer to names of computers in the Metacentrum organization. See

<https://metavo.metacentrum.cz/pbsmon2/hardware>

Output files are numbered according to number of parallel runs of the tool. The numbers represent time taken of single guess, you can use Datamash or other tool to calculate averages, they are already calculated in the file called stats.

## Argon2-gpu tool

This folder contains results obtained through Argon2-gpu-bench tool.

 Laptop and raspberry folder contain particular results, see the benchmarking folder or the thesis for parameters of devices.

Names of subfolders refer to names of computers in the Metacentrum organization. See

<https://metavo.metacentrum.cz/pbsmon2/hardware>

Output files are outputs from the Argon2-gpu-bench tool. In case of computing on CPUs, the folder contains files like

- 1.log

- 2.log

- 4.log

- ...

where the number signifies number of hashes computed concurently. Note that the tool cannot use more threads than available, so higher batch sizes than number of threads introduce false sense of faster hashing.

In case of computation on GPU, the file name is composed of following parts:

- master / warpshuffle / warpshuffleshared - signifies the GPU kernel used, see readme for Argon2-gpu for explanation

- oneshot - if present, signifies that the "one-shot" GPU kernel type was used. If not present, "bi-segment" kernel type was used. One-shot computes all Argon2 slices in parallel, bi-segment type computes them sequentially.

- precomputes - if present, then Argon2-gpu tool precomputes values j1 and j2 for Argon2i and Argon2id, see description of Argon2 for more info. If not present, this mode was not used.

- number - signifies number of parallel independent Argon2 computations performed on the GPU. I tried to always to utilize maximum GPU memory.



