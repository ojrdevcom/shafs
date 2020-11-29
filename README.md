# shafs

SHA256 hash a directory tree, saving paths, file sizes and hashes to a SQLite database

`shafs` can be used to find duplicate files, sort files by size and so forth


## Installation

    git clone https://github.com/ojrdevcom/shafs.git
    autoconf
    automake
    ./configure
    make
    sudo make install



## Usage

    shafs <src_dr> <sqlite_file>



## Example

    shafs /usr/lib ~/usrlib_hashed.db



## Finding Duplicate eBooks

    $ shafs ~/ebooks/ ~/ebooks.db
    $ sqlite3 ~/ebooks.db
    sqlite> select filepath, count(*) as duplicates from shafs group by filehash having duplicates > 0 order by duplicates desc;



## Finding Largest Files

    sqlite> select * from shafs order by filesize desc;



# License

shafs is released under the terms of the MIT license, see attached /LICENSE file




# Peace :)

Shoutouts:

* https://crypto.bi (English)
* https://decenbr.com (Portuguese)
