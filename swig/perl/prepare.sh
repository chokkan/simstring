#!/bin/sh
# $Id:$

ln -s ../export.cpp
ln -s ../export.h
ln -s ../export.i

if [ "$1" = "--swig" ];
then
    perl Makefile.PL
fi
