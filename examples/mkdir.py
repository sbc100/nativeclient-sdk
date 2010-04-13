import sys
import os

for arg in sys.argv:
	if not os.path.exists(arg):
		os.makedirs(arg)