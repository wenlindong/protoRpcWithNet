#!/bin/bash
set -e

rm -rf `pwd`/build/*
rm -rf `pwd`/lib/*
rm -rf `pwd`/bin/provider
rm -rf `pwd`/bin/consumer
cd `pwd`/build &&
	cmake .. &&
	make
cd ..
cp -r `pwd`/include `pwd`/lib