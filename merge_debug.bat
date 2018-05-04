copy /B /Y out\debug_x86\obj\talk\ics\ics.lib dist\Debug\
cd scripts
call python merge_libs_win.py --scheme debug
cd ../dist
del ics-debug.lib
rename ics_dist.lib ics-debug.lib
cd ..