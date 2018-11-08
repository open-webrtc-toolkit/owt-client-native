Package Description
====================
1. gendoc.sh
--------------
Description: Run this script to generate doxygen online document.
It will output the document in a package named 'html' and open 'index.html' in that package to access the index.
Example Usage: ./gendoc.sh
Note: before running this script, you should install doxygen first.
See instruction at https://www.stack.nl/~dimitri/doxygen/manual/install.html
For Ubuntu, you can directly run the following commands to install doxygen under version 1.8.6.
    sudo apt-get update
    sudo apt-get install doxygen
2. cpp.md
--------------
Description: The index markdown file for C++ sdk.
3. doxygen_c++.conf
--------------
The configure file to generate c++ document.
(doxygen related resources are in package 'doxygen' under the parent path.)
