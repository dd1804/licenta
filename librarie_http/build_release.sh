#!/bin/bash

APPLICATION_NAME="aplicatie"

echo
echo "Building network library to build/release/libs ..."

make --file=libs/network/prj/Makefile RELEASE=1

if [ $? -ne 0 ]; then
  exit $?
fi

echo "Static lib network build successfull !"
echo

echo
echo "Building $APPLICATION_NAME to build/release ..."

make --file=prj/Makefile APPNAME=$APPLICATION_NAME RELEASE=1

if [ $? -eq 0 ]; then
  echo "Build successfull !"
  echo
else
  exit $?
fi
