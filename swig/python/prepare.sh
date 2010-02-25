#!/bin/sh

ln -s ../export.cpp
ln -s ../export.h
ln -s ../export.i
swig -c++ -python -o export_wrap.cpp export.i

