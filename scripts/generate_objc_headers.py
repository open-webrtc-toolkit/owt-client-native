# Copyright (C) <2018> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

'''Script to generate Objective-C headers.

It generates Objective-C headers without building SDK framework. So it can be run on multiple platfomrs.
'''

import argparse
import os
import sys

import build

def generate_headers(target_folder):
  if not os.path.exists(target_folder):
    os.makedirs(target_folder)
  build.copyheaders(target_folder)

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_folder', default='out/headers', help='Path for generated headers.')
  opt=parser.parse_args()
  target_folder=os.path.join(os.getcwd(), opt.target_folder)
  generate_headers(target_folder)

if __name__ == '__main__':
  sys.exit(main())
