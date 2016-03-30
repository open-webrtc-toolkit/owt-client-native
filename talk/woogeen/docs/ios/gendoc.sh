#!/bin/bash

#COPY HEADER
python copyheader.py
#GENERATE
rm -rf html
doxygen doxygen_ios.conf

