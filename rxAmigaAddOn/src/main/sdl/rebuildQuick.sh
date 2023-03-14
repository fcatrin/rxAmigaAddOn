#!/bin/sh

rm project
ln -s .. project
./buildQuick.sh
rm project
