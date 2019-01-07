#!/bin/sh

cd /home/bhart/src/repos/echo
echo Building client..
cd /home/bhart/src/repos/echo/client
./build.sh
echo Building server...
cd /home/bhart/src/repos/echo/server
./build.sh
cd /home/bhart/src/repos/echo
echo Project 'echo' - Build complete!

