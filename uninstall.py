import os
from distutils.sysconfig import get_python_lib

top = get_python_lib() + '\eispice'

print top

for root, dirs, files in os.walk(top, topdown=False):
	for name in files:
		os.remove(os.path.join(root, name))

