@echo off
setlocal enabledelayedexpansion

echo === Interval Cloud / Falcor Bootstrap ===
echo.

REM 1) Ensure Falcor's own submodules are fetched (Falcor depends on them)
echo [1/4] Initializing submodules...
git submodule update --init --recursive
if errorlevel 1 (
  echo ERROR: Failed to initialize submodules. Ensure Git is installed and you have network access.
  exit /b 1
)

REM 2) Generate VS2022 solution (Falcor helper)
echo [2/4] Generating Visual Studio 2022 solution...
call setup_vs2022.bat
if errorlevel 1 (
  echo ERROR: setup_vs2022.bat failed. Ensure Visual Studio 2022 and Windows SDK are installed.
  exit /b 1
)

REM 3) Build Release
echo [3/4] Building (Release)...
cmake --build build\windows-vs2022 --config Release
if errorlevel 1 (
  echo ERROR: Build failed. Check the output above for details.
  exit /b 1
)

REM 4) Done
echo [4/4] Success!
echo Binaries are under: build\windows-vs2022\bin\Release
echo.
echo Next: Open build\windows-vs2022\Falcor.sln in VS2022 and run your sample when it exists.
echo (For now, try running existing tools like Mogwai or RenderGraphEditor if present.)
echo.
endlocal
