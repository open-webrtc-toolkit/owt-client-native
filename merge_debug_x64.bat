copy /B /Y out\debug_x64\obj\talk\ics\ics.lib dist\Debug_x64\
cd scripts
call python merge_libs_win.py --scheme debug_x64
cd ../dist
del ics-debug.lib
rename ics_dist.lib ics-debug.lib
cd ..