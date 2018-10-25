copy /B /Y out\debug_x86\obj\talk\oms\oms.lib dist\Debug\
cd scripts
call python merge_libs_win.py --scheme debug
cd ../dist
del oms-debug.lib
rename oms_dist.lib oms-debug.lib
cd ..