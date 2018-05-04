copy /B /Y out\release_x64\obj\talk\ics\ics.lib dist\Release_x64\
cd scripts
call python merge_libs_win.py --scheme release_x64
cd ../dist
del ics-release.lib
rename ics_dist.lib ics-release.lib
cd ..