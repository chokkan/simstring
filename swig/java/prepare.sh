#!/bin/sh

ln -s ../export.cpp
ln -s ../export.h
ln -s ../export.i

if [ "$1" = "--swig" ];
then
    mkdir simstring
    swig -c++ -java -package simstring -outdir simstring export.i
fi
