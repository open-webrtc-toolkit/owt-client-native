rem Copyright (C) <2018> Intel Corporation
rem
rem SPDX-License-Identifier: Apache-2.0

copy /B /Y out\release_x86\obj\talk\oms\oms.lib dist\Release\
cd scripts
call python merge_libs_win.py --scheme release
cd ../dist
del oms-release.lib
rename oms_dist.lib oms-release.lib
cd ..