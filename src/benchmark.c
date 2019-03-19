#include<stdio.h>
#include<libcryptsetup.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<getopt.h>


int main(int argc, char *argv[]) {
//    struct crypt_device *cd = NULL;
    /*if (argc != 4) {
        printf ("wrong number of arguments supplied.\nExpected time, memory, threads.\n");
        return 1;
    }*/
    unsigned int time = 0;
    unsigned int memory = 1048576; //LUKS2 default
    unsigned short int threads = 4;
    unsigned short int repetitions = 0;
    unsigned short int csvoutput = 0;
    int c;
    while ((c = getopt(argc, argv, "t:p:m:r:c")) != -1) {
        switch (c) {
            case 't': {
                time = atoi(optarg);
                break;
            }
            case 'm': {
                memory = atoi(optarg);
                break;
            }
            case 'p': {
                threads = atoi(optarg);
                break;
            }
            case 'r': {
                repetitions = atoi(optarg);
                break;
            }
            case 'c': {
                csvoutput = 1;
                break;
            }
            default: {abort();}
        }
    }
    //printf ("%u %u %u %u\n", time, memory, threads, repetitions);
    if ((time == 0) || (memory == 0) || (threads == 0) || (repetitions == 0)) {
        printf ("Invalid arguments provided.\nRequired arguments:\n-m memory limit in kb, default 1048576\n-p number of parallel processes, default 4\n-t required unlocking time\n-r number of repetitions of the benchmark\n-c turn on csv output\n");
        return 1;
    }
/*    int rt = crypt_init(&cd, NULL);
    if (rt != 1) {
        printf ("Error initialising crypt.\n");
        return rt;
    }*/
    if (csvoutput == 1) {
        printf("iterations\tmemory\tthreads\n");
    }
    for (unsigned int i = 0; i < repetitions; i++) {
        struct crypt_pbkdf_type pbkdf = {
            .type = CRYPT_KDF_ARGON2I,
            .time_ms = time,
            .max_memory_kb = memory,
            .parallel_threads = threads,
        };
        int rt = crypt_benchmark_pbkdf(NULL, &pbkdf, "ahojahoj", 8, "0123456789abcdef0123456789abcdef", 32, 256, NULL, NULL);
        if (rt != 0) {
                printf ("Error while benchmarking.\n%s\n", strerror(rt));
                return rt;
        }
        if (csvoutput == 1) {
                printf ("%u\t%u\t%u\n", pbkdf.iterations, pbkdf.max_memory_kb, pbkdf.parallel_threads);
        }
        else {
            printf ("%4u iterations, %5u memory, %1u parallel threads\n", pbkdf.iterations, pbkdf.max_memory_kb, pbkdf.parallel_threads);
        }
    }
    return 0;
}
