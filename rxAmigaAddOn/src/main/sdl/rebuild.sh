#!/bin/sh

rm project
ln -s .. project
./build.sh
rm project
