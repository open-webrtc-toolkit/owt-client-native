# Copyright (C) <2020> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

'''Script for build in continuous integration environment.

It synchronizes code, and builds SDK. Please run this script with python 3.4 or
newer on Ubuntu 22.04, other platforms are not supported yet.

It's expected to be ran on continuous integration machines and nightly build
machines.
'''

import subprocess
import sys
from pathlib import Path

SRC_PATH = Path(__file__).resolve().parents[2]
GIT_BIN = 'git.bat' if sys.platform == 'win32' else 'git'
GCLIENT_BIN = 'gclient.bat' if sys.platform == 'win32' else 'gclient'

def sync():
    if subprocess.call([GCLIENT_BIN, 'sync', '--reset', '--nohooks'], cwd=SRC_PATH, shell=False):
        return False
    return True

def run_hooks():
    if subprocess.call([GCLIENT_BIN, 'runhooks'], cwd=SRC_PATH, shell=False):
        return False
    return True


def build():
    if subprocess.call(['python3', SRC_PATH/'scripts'/'build_linux.py', '--gn_gen', '--sdk', '--output_path', 'dist', '--scheme', 'release', '--shared', '--use_gcc', '--arch', 'x64', '--cloud_gaming']):
        return False
    return True


def main():
    if not sync():
        return 1
    if not run_hooks():
        return 1
    if not build():
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())