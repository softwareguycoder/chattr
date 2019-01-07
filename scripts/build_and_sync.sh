#!/bin/sh

cd /home/bhart/src/repos/echo/scripts
./build_all.sh
cd /home/bhart/src/repos/echo
./scripts/sync.sh "$1"
echo Done!
