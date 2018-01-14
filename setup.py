from distutils.core import setup, Extension
from distutils.sysconfig import get_python_lib, get_python_inc
import os
import subprocess

# This seems kind of arbitrary, is there a better way ?
numpy_include = get_python_lib(True) + '/numpy/core/include'

if os.name == 'nt':
    extra_compile_args = ["-mnop-fun-dllimport"]
else:
    extra_compile_args = []

# set fortran library depending on type of compiler
try:
    if os.environ['FC'] == 'g77':
        libfortran = 'g2c'
    else:
        libfortran = 'gfortran'
except KeyError:
    libfortran = 'gfortran'

# run the Makefile
subprocess.Popen(['make','libs']).wait()

simulator = Extension('simulator_',
    define_macros = [('LIBNAME', 'simulator')],
    include_dirs = ['./include', numpy_include],
    sources = ['./module/simulatormodule.c'],
    library_dirs=['./libs'],
    libraries=['simulator', 'superlu', 'lapack', 'blas', 'toms', 'cephes',
            'calc', 'data', libfortran],
    extra_compile_args = extra_compile_args)

setup(name = 'eispice',
    version = '0.11.6',
    description = 'eispice Circuit Simulator',
    author = 'Charles Eidsness',
    author_email = 'charles@thedigitalmachine.net',
    url = 'http://www.thedigitalmachine.net/eispice.html',
    license = "GPL",
    ext_modules = [simulator],
    package_dir={'': 'module'},
    packages=[''],
    extra_path = 'eispice')

