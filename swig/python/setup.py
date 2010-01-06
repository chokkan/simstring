#!/usr/bin/env python

"""
setup.py file for SWIG example
"""

import os; os.environ['CC'] = 'g++'; os.environ['CXX'] = 'g++';
os.environ['CPP'] = 'g++'; os.environ['LDSHARED'] = 'g++'

from distutils.core import setup, Extension

simstring_module = Extension(
    '_simstring',
    sources = ['../export_wrap.cpp', '../export.cpp'],
    include_dirs=['/home/users/okazaki/projects/dastring/include'],
    swig_opts=['-c++'],
    )

setup(
    name = 'simstring',
    version = '0.1',
    author = 'Naoaki Okazaki',
    description = """Python module for simstring""",
    ext_modules = [simstring_module],
    py_modules = ["simstring"],
    )

