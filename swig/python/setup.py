#!/usr/bin/env python

"""
setup.py file for SWIG example
"""

import sys
import os
import os.path
import re

def get_rootdir():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
def get_includedir():
    return os.path.join(get_rootdir(), 'include')

def get_swigdir():
    return os.path.join(get_rootdir(), 'swig')

def get_version():
    with open(os.path.join(get_rootdir(), 'configure.in')) as f:
        for line in f:
            m = re.search(r'AM_INIT_AUTOMAKE\([^,]*,\s*([^)]+)\)', line)
            if m:
                return m.group(1)

import os; os.environ['CC'] = 'g++'; os.environ['CXX'] = 'g++';
os.environ['CPP'] = 'g++'; os.environ['LDSHARED'] = 'g++'

from distutils.core import setup, Extension
from distutils.command import build_ext as build_ext_module

class build_ext(build_ext_module.build_ext):
    def run(self):
        prepare_script = os.path.abspath(os.path.join(os.path.dirname(__file__), 'prepare.sh'))
        assert os.system(prepare_script + ' --swig') == 0
        build_ext_module.build_ext.run(self)

simstring_module = Extension(
    '_simstring',
    sources = [
        'export.cpp',
        'export_wrap.cpp',
        ],
    include_dirs=[get_includedir(),],
    extra_link_args=['-shared'],
    language='c++',
    )

setup(
    name = 'simstring',
    version = get_version(),
    author = 'Naoaki Okazaki',
    description = """SimString Python module""",
    ext_modules = [simstring_module],
    py_modules = ["simstring"],
    cmdclass = {'build_ext': build_ext,},
    )

