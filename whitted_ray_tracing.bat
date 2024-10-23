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
    del vec3.o
    exit /b %errorlevel%
)

:: Compile material.cpp
g++ -std=c++11 -c material.cpp -o material.o
if %errorlevel% neq 0 (
    echo Compilation of material.cpp failed.
    del material.o
    del geometry.o
    del vec3.o
    exit /b %errorlevel%
)

:: Compile primitive_tree.cpp
g++ -std=c++11 -c primitive_tree.cpp -o primitive_tree.o
if %errorlevel% neq 0 (
    echo Compilation of primitive_tree.cpp failed.
    del primitive_tree.o
    del material.o
    del geometry.o
    del vec3.o
    exit /b %errorlevel%
)

:: Compile bbox.cpp
g++ -std=c++11 -c bbox.cpp -o bbox.o
if %errorlevel% neq 0 (
    echo Compilation of bbox.cpp failed.
    del bbox.o
    del primitive_tree.o
    del material.o
    del geometry.o
    del vec3.o
    exit /b %errorlevel%
)

:: Compile optics.cpp
g++ -std=c++11 -c optics.cpp -o optics.o
if %errorlevel% neq 0 (
    echo Compilation of optics.cpp failed.
    del optics.o
    del bbox.o
    del primitive_tree.o
    del material.o
    del geometry.o
    del vec3.o
    exit /b %errorlevel%
)

:: Compile and link whitted_ray_tracing.cpp with all object files
g++ -std=c++11 whitted_ray_tracing.cpp geometry.o material.o primitive_tree.o bbox.o optics.o vec3.o -o whitted_ray_tracing.exe
if %errorlevel% neq 0 (
    echo Compilation of whitted_ray_tracing.cpp failed.
    del optics.o
    del bbox.o
    del primitive_tree.o
    del material.o
    del geometry.o
    del vec3.o
    exit /b %errorlevel%
)

echo Compilation successful.

:: Run the executable
whitted_ray_tracing.exe
if %errorlevel% neq 0 (
    echo Executable failed to run.
    del whitted_ray_tracing.exe
    del optics.o
    del bbox.o
    del primitive_tree.o
    del material.o
    del geometry.o
    del vec3.o
    exit /b %errorlevel%
)

echo Executable ran successfully.

:: Clean up
del whitted_ray_tracing.exe
del optics.o
del bbox.o
del primitive_tree.o
del material.o
del geometry.o
del vec3.o

:: Run the Python script
py main.py
if %errorlevel% neq 0 (
    echo Python script failed.
    exit /b %errorlevel%
)

echo Python script ran successfully.