#!/bin/sh -xe
# $Id:$

ln -sf ../export.cpp
ln -sf ../export.h
ln -sf ../export.i

if [ "$1" = "--swig" ];
then
    swig -c++ -python -o export_wrap.cpp export.i
fi
