#!/usr/bin/env python

import os
import sys

from setuptools import setup, find_packages

os.chdir(os.path.dirname(sys.argv[0]) or ".")

setup(
    name="pytrace",
    version="0.1",
    description="Wrapper for libtrace using CFFI",
    long_description=open("README.rst", "rt").read(),
    url="",
    author="Yindong Xiao",
    author_email="xydarcher@uestc.edu.cn",
    classifiers=[
        "Development Status :: 4 - Beta",
        "License :: OSI Approved :: BSD License",
    ],
    packages=find_packages(),
    install_requires=["cffi>=1.0.0"],
    setup_requires=["cffi>=1.0.0"],
    cffi_modules=[
        "./pytrace/build_pytrace.py:ffi",
    ],
)
