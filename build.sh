#!/bin/sh

mkdir -p m4
autoreconf --install
./configure
make

# uncomment to install
# sudo make install