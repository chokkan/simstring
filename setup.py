#!/usr/bin/env python

"""
setup.py file for SWIG example
"""

import sys
import os.path

def get_rootdir():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
def get_includedir():
    return '.'

def get_swigdir():
    return os.path.join(get_rootdir(), 'swig')

#import os; os.environ['CC'] = 'g++'; os.environ['CXX'] = 'g++';
#os.environ['CPP'] = 'g++'; os.environ['LDSHARED'] = 'g++'

from distutils.core import setup, Extension

if sys.platform.startswith("darwin"):
    libs = ['-liconv']
else:
    # need iconv too but without proper -L adding -liconv here won't always work
    libs = []

simstring_module = Extension(
    '_simstring',
    sources = [
        'export.cpp',
        'export_wrap.cpp',
        ],
    include_dirs=[get_includedir(),],
    extra_link_args=libs,
    language='c++',
    )

setup(
    name = 'simstring',
    version = '1.1',
    author = 'Naoaki Okazaki',
    description = """SimString Python module""",
    ext_modules = [simstring_module],
    py_modules = ["simstring"],
    )

