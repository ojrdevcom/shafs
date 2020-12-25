
/*

    shafs_slurp_file - Read file into char *

    Author: @ojrdev
    https://ojrdev.com
    https://crypto.bi 
    https://decenbr.com

    License: See LICENSE file in this distribution    

*/

#include "shafs_slurp_file.h"

extern char shafs_verbose;

char *shafs_slurp_file(char *fil, struct stat *st_ifil) {

    FILE *f;
    size_t ret;
    char *buf = calloc(st_ifil->st_size, 1);

    if (!buf) {
        fprintf(stderr, "shafs_slurp_file: FATAL: Cannot allocate %ld bytes for file %s\n", st_ifil->st_size, fil);
        exit(EXIT_FAILURE);
    }

    f = fopen(fil, "rb");
    if (!f) {
        if (shafs_verbose)
            fprintf(stderr, "shafs_slurp_file: Cannot open file %s\n", fil);  
        return NULL;        
    }

    ret = fread(buf, 1, st_ifil->st_size, f);
    fclose(f);

    if (ret != st_ifil->st_size) {

        if (shafs_verbose) {
            fprintf(stderr, "shafs_slurp_file: ERROR Reading file %s. Read %lu expected %lu\n", fil, ret, st_ifil->st_size);
        }
            
        free(buf);        
        return NULL;
    }

    return buf;
}