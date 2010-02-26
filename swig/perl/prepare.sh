#!/bin/sh

ln -s ../export.cpp
ln -s ../export.h
ln -s ../export.i
swig -c++ -perl -o export_wrap.cpp export.i

