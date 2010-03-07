#!/bin/sh
# $Id:$

ln -s ../export.cpp
ln -s ../export.h
ln -s ../export.i

if [ "$1" = "--swig" ];
then
    swig -c++ -python -o export_wrap.cpp export.i
fi
