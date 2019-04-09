#include<stdio.h>
#include<libcryptsetup.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<getopt.h>





int main(int argc, char *argv[]) {
    struct crypt_device *cd = NULL;
    unsigned int time = 0;
    unsigned int memory = 1048576; //LUKS2 default
    unsigned short int threads = 4;
    unsigned short int repetitions = 0;
    unsigned short int csvoutput = 0;
    int c;
    struct crypt_pbkdf_type* input_pbkdf = NULL; //pbkdf with parameters set with input values
    const struct crypt_pbkdf_type* modified_pbkdf = NULL; //will show parameters modified by crypt_init_pbkdf (lowered memory)
    struct crypt_pbkdf_type *pbkdf = NULL; //pbkdf to be fed in benchmark with final parameters
    pbkdf = malloc(sizeof(struct crypt_pbkdf_type));
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
    int rt = crypt_init(&cd, NULL);
    if (rt != 0) {
        printf ("Error initialising crypt. %d\n", rt);
        return rt;
    }
    //allocating memory for pbkdfs
    input_pbkdf = malloc(sizeof(struct crypt_pbkdf_type));
    pbkdf = malloc(sizeof(struct crypt_pbkdf_type));
    input_pbkdf->type = CRYPT_KDF_ARGON2I;
    input_pbkdf->time_ms = time;
    input_pbkdf->max_memory_kb = memory;
    input_pbkdf->parallel_threads = threads;
    if (csvoutput == 1) {
        printf("iterations\tmemory\tthreads\n");
    }
    for (unsigned int i = 0; i < repetitions; i++) {
        rt = crypt_set_pbkdf_type(cd, input_pbkdf);
        if (rt != 0) {break;}
        modified_pbkdf = crypt_get_pbkdf_type(cd);
        //printf ("%u memory after correction\n", modified_pbkdf->max_memory_kb);
        //copying values modified by benchmark to final pbkdf struct to avoid warning about dropping const qualifier
        pbkdf->type = modified_pbkdf->type;
        pbkdf->time_ms = modified_pbkdf->time_ms;
        pbkdf->max_memory_kb = modified_pbkdf->max_memory_kb;
        pbkdf->parallel_threads = modified_pbkdf->parallel_threads;
        int rt = crypt_benchmark_pbkdf(cd, pbkdf, "ahojahoj", 8, "0123456789abcdef0123456789abcdef", 32, 256, NULL, NULL);
        if (rt != 0) {
                printf ("Error while benchmarking.\n%s\n", strerror(rt));
                return rt;
        }
        if (csvoutput == 1) {
            printf ("%u\t%u\t%u\n", pbkdf->iterations, pbkdf->max_memory_kb, pbkdf->parallel_threads);
        }
        else {
            printf ("%u iterations, %u memory, %u parallel threads\n", pbkdf->iterations, pbkdf->max_memory_kb, pbkdf->parallel_threads);
        }
    }
    crypt_free(cd);
    free(input_pbkdf);
    free(pbkdf);
    return 0;
}
