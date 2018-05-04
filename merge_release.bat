copy /B /Y out\release_x86\obj\talk\ics\ics.lib dist\Release\
cd scripts
call python merge_libs_win.py --scheme release
cd ../dist
del ics-release.lib
rename ics_dist.lib ics-release.lib
cd ..