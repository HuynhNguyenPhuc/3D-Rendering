@echo off

:: Compile the geometry.cpp, bbox.cpp, data_loader.cpp and kdtree.cpp files
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

g++ -std=c++11 -c data_loader.cpp -o data_loader.o
if %errorlevel% neq 0 (
    echo Compilation of data_loader.cpp failed.
    del data_loader.o
    exit /b %errorlevel%
)

g++ -std=c++11 -c kdtree.cpp -o kdtree.o
if %errorlevel% neq 0 (
    echo Compilation of kdtree.cpp failed.
    del kdtree.o
    exit /b %errorlevel%
)

:: Compile and link laurel.cpp with all object files
g++ -std=c++11 laurel.cpp geometry.o bbox.o data_loader.o kdtree.o -o laurel.exe
if %errorlevel% neq 0 (
    echo Compilation of laurel.cpp failed.
    del geometry.o
    del bbox.o
    del data_loader.o
    del kdtree.o
    exit /b %errorlevel%
)

echo Compilation successful.

:: Run the executable
laurel.exe
if %errorlevel% neq 0 (
    echo Executable failed to run.
    del laurel.exe
    del geometry.o
    del bbox.o
    del data_loader.o
    del kdtree.o
    exit /b %errorlevel%
)

echo Executable ran successfully.

:: Clean up
del laurel.exe
del geometry.o
del bbox.o
del data_loader.o
del kdtree.o

:: Run the Python script
py main.py
if %errorlevel% neq 0 (
    echo Python script failed.
    exit /b %errorlevel%
)

echo Python script ran successfully.