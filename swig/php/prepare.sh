#!/bin/sh

ln -s ../export.cpp
ln -s ../export.h
ln -s ../export.i

if [ "$1" = "--swig" ];
then
    swig -c++ -php -prefix Simstring_ -o export_wrap.cpp export.i
fi
