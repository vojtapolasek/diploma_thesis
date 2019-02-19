/*
 * Example of LUKS/LUKS2/TrueCrypt password dictionary search
 *
 * Copyright (C) 2012 Milan Broz <gmazyland@gmail.com>
 * * modified by Vojtěch POlášek <vojtech.polasek@gmail.com> 2019
 *
 * Run this (for LUKS as root),
 * e.g. ./crypt_dict test.img /usr/share/john/password.lst 4
 *
 * This program is free software; you can redistribute it and/or
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

#define MAX_LEN 512

static enum { LUKS, TCRYPT, LUKS2 } device_type;

static void check(struct crypt_device *cd, const char *pwd_file, unsigned my_id, unsigned max_id)
{
    FILE *f;
    int len, r = -1;
    unsigned long line = 0;
    char pwd[MAX_LEN];

    if (fork())
        return;

    /* open password file, now in separate process */
    f = fopen(pwd_file, "r");
    if (!f) {
        printf("Cannot open %s.\n", pwd_file);
        exit(EXIT_FAILURE);
    }

    while (fgets(pwd, MAX_LEN, f)) {

        /* every process tries N-th line, skip others */
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

         //printf("%d: checking %s\n", my_id, pwd);
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
            printf("Found passphrase for slot %d: \"%s\"\n", r, pwd);
            break;
        }
    }

    fclose(f);
    crypt_free(cd);
    exit(r >= 0 ? 2 : EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int i, status, procs = 4;
    char inputfile[512];
    char pwdfile[512];
    char type[16];
    char c;
    struct crypt_device *cd;
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


    /*if (argc < 4 || argc > 5) {
        printf("Use: %s luks|tcrypt <device|file> <password file> [#processes] %d\n", argv[0], argc);
        exit(EXIT_FAILURE);
    }*/

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

    /* crypt_set_debug_level(CRYPT_DEBUG_ALL); */

    /*
     * Need to create temporary keyslot device-mapper devices and allocate loop if needed,
     * so root is required here.
     */
/*    if (getuid() != 0) {
        printf("You must be root to run this program.\n");
                exit(EXIT_FAILURE);
    }*/

    printf ("%d threads.\n", procs);

    /* signal all children if anything happens */
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    setpriority(PRIO_PROCESS, 0, -5);

    /* we are not going to modify anything, so common init is ok */
    if (crypt_init(&cd, inputfile) ||
        (device_type == LUKS && crypt_load(cd, CRYPT_LUKS1, NULL)) ||
        (device_type == LUKS2 && crypt_load(cd, CRYPT_LUKS2, NULL))
    ) {
        printf("Cannot open %s.\n", inputfile);
        exit(EXIT_FAILURE);
    }

    /* run scan in separate processes, it is up to scheduler to assign CPUs inteligently */
    for (i = 0; i < procs; i++)
        check(cd, pwdfile, i, procs);

    /* wait until at least one finishes with error or status 2 (key found) */
    while (wait(&status) != -1 && WIFEXITED(status)) {
        if (WEXITSTATUS(status) == EXIT_SUCCESS)
            continue;
        /* kill rest of processes */
        kill(0, SIGHUP);
        /* not reached */
        break;
    }
    exit(0);
}
