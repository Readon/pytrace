from distutils.core import setup
from Cython.Build import cythonize

setup(
      name = 'Wrapper for libtrace, a network packet parser.',
      ext_modules = cythonize("pytrace.pyx"),

)
