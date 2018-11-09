copy /B /Y out\release_x64\obj\talk\oms\oms.lib dist\Release_x64\
cd scripts
call python merge_libs_win.py --scheme release_x64
cd ../dist
del oms-release.lib
rename oms_dist.lib oms-release.lib
cd ..