@echo off

g++ -o main.exe main.cpp

if %errorlevel%==0 (
    echo Compilation successful.

    main.exe
    del main.exe

    if %errorlevel%==0 (
        echo Executable ran successfully.
        
        py main.py
        if %errorlevel%==0 (
            echo Python script ran successfully.
        ) else (
            echo Python script failed.
        )
    ) else (
        echo Executable failed to run.
    )
) else (
    echo Compilation failed.
)