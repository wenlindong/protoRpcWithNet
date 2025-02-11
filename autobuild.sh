#!/bin/bash
set -e

rm -rf `pwd`/build/*
rm -rf `pwd`/lib/*
rm -rf `pwd`/bin/provider
rm -rf `pwd`/bin/consumer

if [ ! -d "build" ]; then
    mkdir build
fi

cd `pwd`/build &&
	cmake .. &&
	make
cd ..
cp -r `pwd`/include `pwd`/lib
