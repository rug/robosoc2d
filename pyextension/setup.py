import os
from setuptools import setup, Extension

module = Extension('robosoc2d', sources=['robosoc2dmodule.cpp'], language='c++', extra_compile_args=['-std=c++17', '-v']) #python version
#module = Extension('robosoc2d', sources=['robosoc2dmodule.cpp'], language='c++', extra_compile_args=['/std:c++17']) # windows version

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(name='robosoc2d',
    version = '1.0.0',
    description = 'Very Simplified 2D Robotic Soccer Simulator',
    author = 'Ruggero Rossi',
    author_email = 'r.rossi@opencomplexity.com',
    url = 'https://www.github.com/rug/robosoc2d',
    long_description = long_description,
    long_description_content_type="text/markdown",
    ext_modules = [module])

# if you have to build this on google Colab, be advised that
# as of February 24th, 2021, installation with setuptools is for some
# reason not working. 
# It compiles and seems to install, module is visible with "!pip freeze"
# but it is not possible to import it.
# A quick solution to that is to not use setuptools but just plain distutils,
# in which case the package is importable !
# so in that case you should substitute all the previous lines with
# the following three (uncomment them):
#
#from distutils.core import setup, Extension
#setup(name="robosoc2d", version="1.0",
#      ext_modules=[Extension("robosoc2d", ["robosoc2dmodule.cpp"], language='c++')])