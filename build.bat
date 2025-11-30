@echo off
REM ============================
REM  Simple build script (Windows)
REM  使用 g++ 编译到 build\sim.exe
REM ============================

setlocal enabledelayedexpansion

REM 如果你用的是 MSYS2/MinGW，请确认 g++.exe 在 PATH 里
set CXX=g++
set CXXFLAGS=-std=c++17 -O2 -Wall -Wextra

set SRC= ^
  main.cpp ^
  Server.cpp ^
  BaseScheduler.cpp ^
  DAScheduler.cpp ^
  Simulation.cpp ^
  DataGenerator.cpp ^
  Metrics.cpp ^
  Config.cpp ^
  ResultWriter.cpp

if not exist build (
  mkdir build
)

echo.
echo === Compiling with %CXX% ===
echo %CXX% %CXXFLAGS% %SRC% -o build\sim.exe
echo.

%CXX% %CXXFLAGS% %SRC% -o build\sim.exe
if errorlevel 1 (
  echo.
  echo *** Build failed ***
  goto :eof
)

echo.
echo *** Build succeeded: build\sim.exe ***
echo You can run:  build\sim.exe [config.txt]
echo.

endlocal
