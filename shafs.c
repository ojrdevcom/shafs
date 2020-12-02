/*
    shafs - SHA256 an entire filesystem, saving hashes to a sqlite database

    Author: @ojrdev
    https://ojrdev.com
    https://crypto.bi 
    https://decenbr.com

    License: See LICENSE file in this distribution    

*/

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <sqlite3.h>
#include <unistd.h>
#include <linux/limits.h>

#include "sha256.h"

static char *shafs_sql_insert = "INSERT INTO shafs (filepath, filehash, filesize) VALUES(?, ?, ?)";
unsigned char shafs_verbose = 0;

static int shafs_usage() {    
    fprintf(stderr, "Usage: shafs <src_dir> <sqlite_file>\n");
    return EXIT_FAILURE;
}

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
        if (shafs_verbose)
            fprintf(stderr, "shafs_slurp_file: ERROR Reading file %s. Read %lu expected %lu\n", fil, ret, st_ifil->st_size);
        free(buf);        
        return NULL;
    }

    return buf;
}

char *shafs_file_hash(char *fil, struct stat *st_ifil) {

    unsigned char *hash_buf = calloc(32, 1);
    char *hash_buf_str = calloc(65, 1);
    char *contents;

    if (!hash_buf) {
        fprintf(stderr, "shafs_file_hash: FATAL: Cannot allocate 32 bytes for SHA256 bytes hash of %s\n", fil);
        exit(EXIT_FAILURE);
    }

    if (!hash_buf_str) {
        fprintf(stderr, "shafs_file_hash: FATAL: Cannot allocate 65 bytes for SHA256 string hash of %s\n", fil);
        exit(EXIT_FAILURE);
    }

    contents = shafs_slurp_file(fil, st_ifil);    

    if (!contents) {
        if (shafs_verbose)
            fprintf(stderr, "shafs_file_hash: Error reading %s\n", fil);
        return NULL;
    }

    SHA256_CTX ctx;
  	sha256_init(&ctx);
	sha256_update(&ctx, (const BYTE *)contents, st_ifil->st_size);
	sha256_final(&ctx, hash_buf);    

    for (int i=0;i<32;i++) {
        int offs = i << 1;
        unsigned char c = hash_buf[i];
        sprintf(hash_buf_str + offs, "%02x", c);
    }

    free(contents);
    free(hash_buf);
    
    return hash_buf_str;
}

void shafs_work_file(char *fil, struct stat *st_ifil, sqlite3 *db){

    if (!S_ISREG(st_ifil->st_mode)) {
        return;
    }

    char *hash = shafs_file_hash(fil, st_ifil);

    if (!hash) {
        return;
    }

    sqlite3_stmt *stmt;

    if (shafs_verbose) {
        printf("%s %s\n", fil, hash);
    }    
            
    int ret = sqlite3_prepare_v2(db, shafs_sql_insert, -1, &stmt, 0);

    if (ret == SQLITE_OK) {        
        sqlite3_bind_text(stmt, 1, fil, strlen(fil), NULL);
        sqlite3_bind_text(stmt, 2, hash, strlen(hash), NULL);
        sqlite3_bind_int(stmt, 3, st_ifil->st_size);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {    
        if (shafs_verbose)
            fprintf(stderr, "shafs_work_file: Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }   

    free(hash);     
}

void shafs_walk_dir(char *fil, struct stat *st_idir, sqlite3 *db) {

    DIR *dir;
    struct dirent *dent;
    int ret = 0;

    if (!S_ISDIR(st_idir->st_mode)) {
        shafs_work_file(fil, st_idir, db);        
        return;
    }

    dir = opendir(fil);    
    if (!dir) {
        if (shafs_verbose)
            fprintf(stderr, "shafs_walk_dir: Cannot opendir %s\n", fil);
        return;
    }

    dent = readdir(dir);
    while (dent) {

        struct stat st;

        if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) {
            dent = readdir(dir);
            continue;
        }

        size_t fil_len = strlen(fil) + strlen(dent->d_name) + 2;

        if (fil_len > PATH_MAX) {
            if (shafs_verbose) {
                printf("ERROR: Filename too long - skipping. Filesystem loop? (%s/%s)\n", fil, dent->d_name);
            }  
            continue;
        }

        char *newf = calloc(fil_len, 1);
        if (!newf) {
            fprintf(stderr, "shafs_walk_dir: FATAL: Out of memory while processing %s\n", fil);
            exit(EXIT_FAILURE);
        }
        
        strcat(newf, fil);
        if (strcmp(fil, "/")) {
            strcat(newf, "/");
        }
        
        strcat(newf, dent->d_name);        
        ret = stat(newf, &st);
        if (ret) {
            fprintf(stderr, "shafs_walk_dir stat %s", newf);
            perror("");
            free(newf);
            dent = readdir(dir);
            continue;
        }

        shafs_walk_dir(newf, &st, db);
        free(newf);
        dent = readdir(dir);
    }
    closedir(dir);
}

int main(int argc, char **argv){

    int c, ret, ix, cnti=0;
    struct stat st_idir;
    sqlite3 *db;

    while ((c = getopt (argc, argv, "v")) != -1) {
        switch (c) {
            case 'v':
                shafs_verbose = 1;
                break;
        }    
    }

    for (ix = optind; ix < argc; ix++) cnti++;
    if (cnti != 2) {
        return shafs_usage();
    }    

    if (strlen(argv[optind]) > PATH_MAX) {
        if (shafs_verbose) {
            printf("FATAL: Source path name too long. (%s)\n", argv[optind+1]);
        }  
        return EXIT_FAILURE;
    }

    if (strlen(argv[optind+1]) > PATH_MAX) {
        if (shafs_verbose) {
            printf("FATAL: Database file path too long. (%s)\n", argv[optind+1]);
        }  
        return EXIT_FAILURE;
    }

    ret = sqlite3_open(argv[optind+1], &db);
    if (ret != SQLITE_OK) {        
        fprintf(stderr, "main: FATAL: Error opening DB %s: %s\n", argv[optind+1], sqlite3_errmsg(db));
        sqlite3_close(db);        
        return EXIT_FAILURE;
    }    

    sqlite3_exec(db, "CREATE TABLE shafs(filepath VARCHAR(4096) PRIMARY KEY, filehash CHAR(32), filesize INTEGER NOT NULL DEFAULT 0)", NULL, NULL, NULL);
    sqlite3_exec(db, "CREATE INDEX by_filehash ON shafs(filehash)", NULL, NULL, NULL);
    sqlite3_exec(db, "CREATE INDEX by_filesize ON shafs(filesize)", NULL, NULL, NULL);

    ret = stat(argv[optind], &st_idir);
    if (ret) {
        perror("Error opening source directory");
        return EXIT_FAILURE;
    }

    shafs_walk_dir(argv[optind], &st_idir, db);
    sqlite3_close(db);

    return EXIT_SUCCESS;
}
