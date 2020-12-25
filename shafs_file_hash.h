#ifndef _SHAFS_FILE_HASH
#define _SHAFS_FILE_HASH

/*

    shafs_file_hash - Produce sha256sum of a file.

    Author: @ojrdev
    https://ojrdev.com
    https://crypto.bi 
    https://decenbr.com

    License: See LICENSE file in this distribution    

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "sha256.h"

#define SHAFS_HASH_LEN 32
#define SHAFS_HASH_STR_LEN 65

char *shafs_file_hash(char *fil, struct stat *st_ifil);

#endif