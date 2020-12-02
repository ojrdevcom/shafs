#!/bin/sh

mkdir -p m4
autoreconf --install
./configure
make
sudo make install