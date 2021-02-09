#!/bin/bash

APPLICATION_NAME="aplicatie"
echo "Cleaning ..."

make --file=prj/Makefile APPNAME=$APPLICATION_NAME DEBUG=1  clean
make --file=prj/Makefile APPNAME=$APPLICATION_NAME RELEASE=1 clean

make --file=libs/network/prj/Makefile DEBUG=1  clean
make --file=libs/network/prj/Makefile RELEASE=1 clean

echo "Finished cleaning !"
