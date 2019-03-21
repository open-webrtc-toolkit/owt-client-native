# Copyright (C) <2018> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# Copyright (c) 2018 Intel Corporation. All Rights Reserved.
import os
import re
import sys
import argparse
HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
def replaceFiles(dirPath):
	for root,dirs,files in os.walk(dirPath):
		for curFile in files:
			print os.path.join(root, curFile)
			f = open(os.path.join(root, curFile), 'r+')
			lines = f.readlines()
			f.seek(0)
			f.truncate()
			for line in lines:
				orig = 'ics'
				replace = 'owt'
				f.write(line.replace(orig, replace))
			f.close();

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('--dir', default='talk', help='root directory to be replaced')
	replace_root_path = os.path.join(HOME_PATH, parser.parse_args().dir)
	print replace_root_path
	replaceFiles(replace_root_path)
	print 'Done'
	return 0

if __name__ == '__main__':
  sys.exit(main())
