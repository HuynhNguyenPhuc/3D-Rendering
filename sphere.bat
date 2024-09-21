@echo off

:: Compile the geometry.cpp and bbox.cpp file
g++ -std=c++11 -c geometry.cpp -o geometry.o
if %errorlevel% neq 0 (
    echo Compilation of geometry.cpp failed.
    del geometry.o
    exit /b %errorlevel%
)

g++ -std=c++11 -c bbox.cpp -o bbox.o
if %errorlevel% neq 0 (
    echo Compilation of bbox.cpp failed.
    del bbox.o
    exit /b %errorlevel%
)

:: Compile and link sphere.cpp
g++ -std=c++11 sphere.cpp geometry.o bbox.o -o sphere.exe
if %errorlevel% neq 0 (
    echo Compilation of sphere.cpp failed.
    del geometry.o
    del bbox.o
    exit /b %errorlevel%
)

:: Compile and link sphere.cpp
g++ -std=c++11 sphere.cpp geometry.o bbox.o -o sphere.exe
if %errorlevel% neq 0 (
    echo Compilation of sphere.cpp failed.
    del geometry.o
    del bbox.o
    exit /b %errorlevel%
)

echo Compilation successful.

:: Run the executable
sphere.exe
if %errorlevel% neq 0 (
    echo Executable failed to run.
    del sphere.exe
    del geometry.o
    del bbox.o
    exit /b %errorlevel%
)

echo Executable ran successfully.

:: Clean up
del sphere.exe
del geometry.o
del bbox.o

:: Run the Python script
py main.py
if %errorlevel% neq 0 (
    echo Python script failed.
    exit /b %errorlevel%
)

echo Python script ran successfully.