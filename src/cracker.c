/*
 * Example of LUKS/LUKS2/TrueCrypt password dictionary search
 *
 * Copyright (C) 2012 Milan Broz <gmazyland@gmail.com>
 * * modified by Vojtěch Polášek <vojtech.polasek@gmail.com> 2019
 *
  This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <libcryptsetup.h>
#include<getopt.h>
#include<pthread.h>
#include<time.h>

#define MAX_LEN 512

//defining structure for passing data to threads
struct targ {const char *disk_file; const char *pwd_file; unsigned my_id; unsigned max_id; struct timespec *averages;};

//global primitives for signaling to the main thread that the passphrase has been found
unsigned short int passphrase_found=0;
pthread_mutex_t pass_mutex;



//defining structure for device type
static enum { LUKS, TCRYPT, LUKS2 } device_type;


//function for computing difference between two time points
struct timespec timediff(struct timespec a, struct timespec b)
{
        struct timespec result;
        if ((b.tv_nsec - a.tv_nsec)<0) {
                result.tv_sec = b.tv_sec - a.tv_sec-1;
                result.tv_nsec = 1000000000 + b.tv_nsec - a.tv_nsec;
        } else {
                result.tv_sec = b.tv_sec - a.tv_sec;
                result.tv_nsec = b.tv_nsec - a.tv_nsec;
        }
        return result;
}

// function for adding two time points
struct timespec timeadd(struct timespec a, struct timespec b) {
    struct timespec result;
    if ((a.tv_nsec + b.tv_nsec) >= 1000000000) {
            result.tv_sec = a.tv_sec + b.tv_sec + 1;
            result.tv_nsec = a.tv_nsec + b.tv_nsec -1000000000;
    }
    else {
            result.tv_sec = a.tv_sec + b.tv_sec;
            result.tv_nsec = a.tv_nsec + b.tv_nsec;
    }
    return result;
}


//function for dividing of two time points
struct timespec timedivide(struct timespec a, unsigned int b) {
    struct timespec result;
    if (a.tv_sec / b < 1) {
            result.tv_sec = 0;
            double tmp = (double)a.tv_sec / b;
            result.tv_nsec = (a.tv_nsec / b) + (tmp * 1000000000);
    }
    else {
            result.tv_sec = a.tv_sec / b;
            result.tv_nsec = a.tv_nsec / b;
    }
    return result;
}

//determine number of digits in number
unsigned int digits (int number) {
    unsigned int result = 1;
    int tmpnum = number;
    while ((int)(tmpnum / 10) != 0) {
        result ++;
        tmpnum /= 10;
    }
    return result;
}

//this function is ran by every thread, tries to guess a passphrase
void* check(void* threadargs)
{
    const char *disk_file;
    const char *pwd_file;
    unsigned my_id; //id of the current thread
    unsigned max_id;
    struct timespec *averages; //array for storing averages
    struct targ* args = (struct targ*) threadargs;
    disk_file = args->disk_file;
    pwd_file = args->pwd_file;
    my_id = args->my_id;
    max_id = args->max_id;
    averages = args->averages;
    free(args);
    FILE *f;
    int len, r = -1;
    unsigned long line = 0;
    struct crypt_device *cd;
    char pwd[MAX_LEN];
    fprintf (stderr, "Thread %d\n", my_id);



    if (crypt_init(&cd, disk_file) ||
            (device_type == LUKS && crypt_load(cd, CRYPT_LUKS1, NULL)) ||
            (device_type == LUKS2 && crypt_load(cd, CRYPT_LUKS2, NULL))
        ) {
            fprintf(stderr, "Cannot open %s.\n", disk_file);
            pthread_exit(NULL);
        }

    fprintf (stderr, "Thread %d ready.\n", my_id);
    /* open password file, now in separate thread */
    f = fopen(pwd_file, "r");
    if (!f) {
        fprintf(stderr, "Cannot open %s.\n", pwd_file);
        pthread_exit(NULL);
    }
    //time measurement
    struct timespec start, end;
    struct timespec cumultime; //cumulating all measurements of this thread
    unsigned int loopcount = 0; //counting number of tries, used to calculate average



    while (fgets(pwd, MAX_LEN, f)) {
        //check if passphrase not yet found
        pthread_mutex_lock(&pass_mutex);
        if (passphrase_found == 1) {
            pthread_mutex_unlock(&pass_mutex);
            break;
        }
        pthread_mutex_unlock(&pass_mutex);

        clock_gettime(CLOCK_MONOTONIC, &start);
        /* every thread tries N-th line, skip others */
        if (line++ % max_id != my_id)
            continue;

        len = strlen(pwd);

        /* strip EOL - this is like a input from tty */
        if (len && pwd[len - 1] == '\n') {
            pwd[len - 1] = '\0';
            len--;
        }

        /* lines starting "#!comment" are comments */
        if (len >= 9 && !strncmp(pwd, "#!comment", 9)) {
            /* printf("skipping %s\n", pwd); */
            continue;
        }

        if ((device_type == LUKS) || (device_type == LUKS2))
            r = crypt_activate_by_passphrase(cd, NULL, CRYPT_ANY_SLOT, pwd, len, 0);
        else if (device_type == TCRYPT) {
            struct crypt_params_tcrypt params = {
                .flags = CRYPT_TCRYPT_LEGACY_MODES,
                .passphrase = pwd,
                .passphrase_size = len,
            };
            r = crypt_load(cd, CRYPT_TCRYPT, &params);
        }
        if (r >= 0) {
            pthread_mutex_lock(&pass_mutex);
            passphrase_found = 1;
            pthread_mutex_unlock(&pass_mutex);
            fprintf(stderr, "Found passphrase for slot %d: \"%s\"\n", r, pwd);
            break;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        struct timespec dif = timediff(start, end);
        //printing in seconds.milliseconds
        printf ("%ld.%09ld\n", dif.tv_sec, dif.tv_nsec);
        cumultime = timeadd(dif, cumultime);
        loopcount += 1;
    }

    fclose(f);
    crypt_free(cd);
    struct timespec avgtime = timedivide(cumultime, loopcount);
    averages[my_id] = avgtime;
    fprintf (stderr, "Thread %d finished\n", my_id);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int i, procs = 4;
    char inputfile[512];
    char pwdfile[512];
    char type[16];
    char c;
    //time measurement
    struct timespec start, end;
    while ((c = getopt(argc, argv, "t:p:i:T:")) != -1) {
        switch (c) {
            case 't': {
                if (optarg) {strncpy(type, optarg, 16);}
                else {printf("Missing the type argument.\n");}
                break;
            }
            case 'i': {
                if (optarg) {strncpy(inputfile, optarg, 512);}
                else {printf("Missing path to the input image file.\n");}
                break;
            }
            case 'p': {
                if (optarg) {strncpy(pwdfile, optarg, 512);}
                else {printf("Missing path to the password file.\n");}
                break;
            }
            case 'T': {
                if (optarg) {procs = atoi(optarg);}
                else {printf("Missing number of threads.\n");}
                break;
            }
            default: {abort();}
        }
    }
    if (procs < 1) {
        printf("Wrong number of processes.\n");
        exit(EXIT_FAILURE);
    }

    if (!strncmp(type, "luks", 5)) {
        device_type = LUKS;
    }
    else if (!strncmp(type, "tcrypt", 7)) {
        device_type = TCRYPT;
    }
    else if (!strncmp(type, "luks2", 6)) {
        device_type = LUKS2;
    }
    else {
        printf("Wrong device type %s.\n", type);
        exit(EXIT_FAILURE);
    }

        fprintf (stderr, "%d threads.\n", procs);

    pthread_t thread_ids[procs];
    //defining array for averages from time measurement
    struct timespec averages[procs];
    clock_gettime(CLOCK_MONOTONIC, &start);

    /* run scan in separate threads, it is up to scheduler to assign CPUs inteligently */
    for (i = 0; i < procs; i++) {
        struct targ *tmptarg = malloc(sizeof(struct targ));
        tmptarg->disk_file = inputfile;
        tmptarg->pwd_file = pwdfile;
        tmptarg->my_id = i;
        tmptarg->max_id = procs;
        tmptarg->averages = averages;
        pthread_create(&thread_ids[i], NULL, check, tmptarg);
    }
    //joining threads
    for (int i = 0; i < procs; i++) {
            pthread_join(thread_ids[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    struct timespec dif = timediff(start, end);
    fprintf (stderr, "total calculation %ld.%09ld s \n", dif.tv_sec, dif.tv_nsec);
    struct timespec cumulaverage; //calculating total average
    cumulaverage.tv_sec = 0;
    cumulaverage.tv_nsec = 0;
    for (int i = 0; i < procs; i++) {
        cumulaverage.tv_sec += averages[i].tv_sec;
        cumulaverage.tv_nsec += averages[i].tv_nsec;
    }
    struct timespec finalaverage = timedivide(cumulaverage, procs);
    fprintf (stderr, "final average %ld.%09ld s\n", finalaverage.tv_sec, finalaverage.tv_nsec);
    pthread_mutex_destroy(&pass_mutex);
    pthread_exit(NULL);
}
