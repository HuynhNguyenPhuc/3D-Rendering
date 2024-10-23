@echo off

:: Compile vec3.cpp
g++ -std=c++11 -c vec3.cpp -o vec3.o
if %errorlevel% neq 0 (
    echo Compilation of vec3.cpp failed.
    del vec3.o
    exit /b %errorlevel%
)

:: Compile geometry.cpp
g++ -std=c++11 -c geometry.cpp -o geometry.o
if %errorlevel% neq 0 (
    echo Compilation of geometry.cpp failed.
    del geometry.o
    exit /b %errorlevel%
)

:: Compile bbox.cpp
g++ -std=c++11 -c bbox.cpp -o bbox.o
if %errorlevel% neq 0 (
    echo Compilation of bbox.cpp failed.
    del bbox.o
    exit /b %errorlevel%
)

:: Compile data_loader.cpp
g++ -std=c++11 -c data_loader.cpp -o data_loader.o
if %errorlevel% neq 0 (
    echo Compilation of data_loader.cpp failed.
    del data_loader.o
    exit /b %errorlevel%
)

:: Compile primitive_tree.cpp
g++ -std=c++11 -c primitive_tree.cpp -o primitive_tree.o
if %errorlevel% neq 0 (
    echo Compilation of primitive_tree.cpp failed.
    del primitive_tree.o
    exit /b %errorlevel%
)

:: Compile optics.cpp
g++ -std=c++11 -c optics.cpp -o optics.o
if %errorlevel% neq 0 (
    echo Compilation of optics.cpp failed.
    del optics.o
    exit /b %errorlevel%
)

:: Compile material.cpp
g++ -std=c++11 -c material.cpp -o material.o
if %errorlevel% neq 0 (
    echo Compilation of material.cpp failed.
    del material.o
    exit /b %errorlevel%
)

:: Compile and link laurel.cpp with all object files
g++ -std=c++11 laurel.cpp vec3.o geometry.o bbox.o data_loader.o primitive_tree.o optics.o material.o -o laurel.exe
if %errorlevel% neq 0 (
    echo Compilation and linking of laurel.cpp failed.
    exit /b %errorlevel%
)

echo Compilation and linking successful.

:: Run the executable
laurel.exe
if %errorlevel% neq 0 (
    echo Executable failed to run.
    exit /b %errorlevel%
)

echo Executable ran successfully.

:: Clean up object files
del vec3.o
del geometry.o
del bbox.o
del data_loader.o
del primitive_tree.o
del optics.o
del material.o
del laurel.exe

:: Run the Python script 
py main.py
if %errorlevel% neq 0 (
    echo Python script failed.
    exit /b %errorlevel%
)

echo Python script ran successfully.