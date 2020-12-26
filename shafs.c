/*
    shafs - SHA256 every file under a filesystem, saving hashes, file sizes and paths to a sqlite database

    Author: @ojrdev
    https://ojrdev.com
    https://crypto.bi 
    https://decenbr.com

    License: See LICENSE file in this distribution    

*/

#include "shafs.h"
#include "shafs_file_hash.h"

static char *shafs_sql_insert = "INSERT INTO shafs (filepath, filehash, filesize) VALUES(?, ?, ?) ON CONFLICT(filepath) DO UPDATE SET filehash = ?, filesize = ?";
unsigned char shafs_verbose = 0;

static int shafs_usage() {    
    fprintf(stderr, "Usage: shafs <src_dir> <sqlite_file>\n");
    return EXIT_FAILURE;
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
        sqlite3_bind_text(stmt, 4, hash, strlen(hash), NULL);
        sqlite3_bind_int(stmt, 5, st_ifil->st_size);
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

    if (shafs_verbose)
        printf("shafs_walk_dir(%s)\n", fil);


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
        ret = lstat(newf, &st);
        if (ret) {            
            if (shafs_verbose) {
                fprintf(stderr, "shafs_walk_dir stat %s: ", newf);
                perror("");
            }
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
    time_t start_time;
    time_t end_time;
    struct tm * start_timeinfo;
    struct tm * end_timeinfo;

    time(&start_time);
    start_timeinfo = localtime(&start_time);
    fprintf(stderr, "Start %s", asctime(start_timeinfo));

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

    // Ignores errors if tables already exist. Should probably improve this.
    sqlite3_exec(db, "CREATE TABLE shafs(filepath VARCHAR(4096) PRIMARY KEY, filehash CHAR(64), filesize INTEGER NOT NULL DEFAULT 0)", NULL, NULL, NULL);
    sqlite3_exec(db, "CREATE INDEX by_filehash ON shafs(filehash)", NULL, NULL, NULL);
    sqlite3_exec(db, "CREATE INDEX by_filesize ON shafs(filesize)", NULL, NULL, NULL);

    ret = lstat(argv[optind], &st_idir);
    if (ret) {
        perror("Error opening source directory");
        return EXIT_FAILURE;
    }

    shafs_walk_dir(argv[optind], &st_idir, db);
    sqlite3_close(db);

    time(&end_time);
    end_timeinfo = localtime(&end_time);
    
    fprintf(stderr, "End   %s", asctime(end_timeinfo));

    return EXIT_SUCCESS;
}
