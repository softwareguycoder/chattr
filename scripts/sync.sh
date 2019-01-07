#!/bin/sh

cd ~/src/repos/echo
git pull origin master
git add .
git stage .
git commit -m "$1"
git push origin master

