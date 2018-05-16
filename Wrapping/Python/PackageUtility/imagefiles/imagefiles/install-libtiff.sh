#!/bin/bash

cd /tmp/archive
tar -xzf tiff-4.0.9.tar.gz -C /tmp/
cd /tmp/tiff-4.0.9
./configure CC=/usr/local/bin/gcc CXX=/usr/local/bin/g++
make
make install
