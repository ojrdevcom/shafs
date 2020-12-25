
#ifndef _SHAFS_SLURP_FILE
#define _SHAFS_SLURP_FILE

/*

    shafs_slurp_file - Read file into a string.

    Author: @ojrdev
    https://ojrdev.com
    https://crypto.bi 
    https://decenbr.com

    License: See LICENSE file in this distribution    

*/

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

char *shafs_slurp_file(char *fil, struct stat *st_ifil);

#endif