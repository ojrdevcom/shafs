/*

    shafs_file_hash - Produce char * sha256sum of a file.

    Author: @ojrdev
    https://ojrdev.com
    https://crypto.bi 
    https://decenbr.com

    License: See LICENSE file in this distribution    

*/

#include "shafs_file_hash.h"
#include "shafs_slurp_file.h"

extern char shafs_verbose;

char *_shafs_file_hash_single_chunk(char *fil, struct stat *st_ifil) {

    unsigned char *hash_buf = calloc(SHAFS_HASH_LEN, 1);
    char *hash_buf_str = calloc(SHAFS_HASH_STR_LEN, 1);
    char *contents;

    if (!hash_buf) {
        fprintf(stderr, "_shafs_file_hash_single_chunk: FATAL: Cannot allocate memory for the hash of %s\n", fil);
        exit(EXIT_FAILURE);
    }

    if (!hash_buf_str) {
        fprintf(stderr, "_shafs_file_hash_single_chunk: FATAL: Cannot allocate memory for the hex string hash of %s\n", fil);
        exit(EXIT_FAILURE);
    }

    contents = shafs_slurp_file(fil, st_ifil);    

    if (!contents) {
        if (shafs_verbose)
            fprintf(stderr, "_shafs_file_hash_single_chunk: Error reading %s\n", fil);
        return NULL;
    }

    SHA256_CTX ctx;
  	sha256_init(&ctx);
	sha256_update(&ctx, (const BYTE *)contents, st_ifil->st_size);
	sha256_final(&ctx, hash_buf);    

    for (int i=0;i<SHAFS_HASH_LEN;i++) {
        int offs = i << 1;
        unsigned char c = hash_buf[i];
        sprintf(hash_buf_str + offs, "%02x", c);
    }

    free(contents);
    free(hash_buf);
    
    return hash_buf_str;
}

char *_shafs_file_hash_stream(char *fil, struct stat *st_ifil) {

    FILE *f;
    size_t ret;    
    unsigned char *hash_buf = calloc(SHAFS_HASH_LEN, 1);
    char *hash_buf_str = calloc(SHAFS_HASH_STR_LEN, 1);
    char *contents;
    SHA256_CTX ctx;

    if (!hash_buf) {
        fprintf(stderr, "_shafs_file_hash_stream: FATAL: Cannot allocate memory for the hash of %s\n", fil);
        exit(EXIT_FAILURE);
    }

    if (!hash_buf_str) {
        fprintf(stderr, "_shafs_file_hash_stream: FATAL: Cannot allocate memory for the hex string hash of %s\n", fil);
        exit(EXIT_FAILURE);
    }

    contents = calloc(SHAFS_HASH_BUFFSIZ, 1); 

    if (!contents) {
        if (shafs_verbose)
            fprintf(stderr, "_shafs_file_hash_stream: Error allocating %u bytes.\n", SHAFS_HASH_BUFFSIZ);
        return NULL;
    }

    f = fopen(fil, "rb");
    if (!f) {
        if (shafs_verbose)
            fprintf(stderr, "_shafs_file_hash_stream: Cannot open file %s\n", fil);  
        return NULL;        
    }

  	sha256_init(&ctx);

    do {
        ret = fread(contents, 1, SHAFS_HASH_BUFFSIZ, f);
        sha256_update(&ctx, (const BYTE *)contents, ret);
    } while(ret == SHAFS_HASH_BUFFSIZ);

    fclose(f);            
	sha256_final(&ctx, hash_buf);    

    for (int i=0;i<SHAFS_HASH_LEN;i++) {
        int offs = i << 1;
        unsigned char c = hash_buf[i];
        sprintf(hash_buf_str + offs, "%02x", c);
    }

    free(contents);
    free(hash_buf);
    
    return hash_buf_str;
}

char *shafs_file_hash(char *fil, struct stat *st_ifil) {

    if (st_ifil->st_size < SHAFS_SINGLE_FILE_SLURP_LIMIT) {
        return _shafs_file_hash_single_chunk(fil, st_ifil);
    }

    return _shafs_file_hash_stream(fil, st_ifil);
}
