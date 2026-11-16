@echo off
echo Compiling MiniGit (C++ OOP Project)...
g++ -Wall -std=c++11 -Iinclude src/main.cpp src/GitObjects.cpp src/BareRemote.cpp src/Repository.cpp -o minigit.exe
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Built minigit.exe successfully!
    echo Run 'minigit.exe' for Interactive Menu or 'minigit.exe help' for CLI commands.
) else (
    echo [ERROR] Compilation failed. Ensure g++ is installed and in your PATH.
)
