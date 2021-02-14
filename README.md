# shafs

SHA256 every file in a directory tree, saving paths, file sizes and hashes to a SQLite database

`shafs` can be used to find duplicate files, sort files by size and so forth

## Dependencies

shafs requires a valid GNU standard C build system and sqlite3 version > 3.24.0*

On Ubuntu:

    sudo apt install sqlite3
    sudo apt install libsqlite3-dev

Please check your distribution for the appropriate packages.

*Note: SQLite3 versions < 3.24.0 will not work unless the SQL queries are modified to not use the [UPSERT](https://www.sqlite.org/draft/lang_UPSERT.html) feature. This might generate lots of duplicates in the DB, which you'll need to take into consideration in your queries.

## Build

    git clone https://github.com/ojrdevcom/shafs.git
    cd shafs
    ./build.sh

## Installation

To install locally, uncomment last line in `build.sh` and re-run it or run :

    sudo make install

## Manual Build + Install

    git clone https://github.com/ojrdevcom/shafs.git
    cd shafs
    mkdir -p m4
    autoreconf --install
    ./configure
    make
    sudo make install    


## Usage

    shafs [-v] <src_dr> <sqlite_file>



## Example

    shafs -v /usr/lib ~/usrlib_hashed.db



## Finding Duplicate eBooks

    $ shafs ~/ebooks/ ~/ebooks.db
    $ sqlite3 ~/ebooks.db
    sqlite> select filepath, count(*) as duplicates from shafs group by filehash having duplicates > 0 order by duplicates desc;



## Finding Largest Files

    sqlite> select * from shafs order by filesize desc;



# License

shafs is released under the terms of the MIT license 

See attached LICENSE file for details



# Peace :)

Author: @ojrdev

Shoutouts:

* https://crypto.bi (English)
* https://decenbr.com (Portuguese)
