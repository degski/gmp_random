:: @ call compile.cmd
@ xcopy .\x64\Release\gmp_random.exe .\ /Y
@ gmp_random.exe | RNG_test-0.94.exe stdin64 -tlmaxonly
