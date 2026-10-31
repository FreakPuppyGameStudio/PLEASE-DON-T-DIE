@echo off
rem ============================================================
rem  PLEASE, DON'T DIE v0.1 - build with MinGW (w64devkit)
rem  Usage (PowerShell or cmd):
rem    .\build_mingw.bat          compile to build_mingw\
rem    .\build_mingw.bat run      compile and run
rem    .\build_mingw.bat dist     compile and make dist\PleaseDontDie-v0.1.zip
rem ============================================================
setlocal
cd /d "%~dp0"

rem Folder with raylib.h and libraylib.a
set "RAYLIB_DIR=C:\raylib\raylib\src"
rem Folder with g++.exe (w64devkit)
set "PATH=C:\raylib\w64devkit\bin;%PATH%"

set "OUT=build_mingw"
set "DIST=dist\PleaseDontDie"

if not exist "%RAYLIB_DIR%\raylib.h" goto no_header
if not exist "%RAYLIB_DIR%\libraylib.a" goto no_lib

if not exist "%OUT%" mkdir "%OUT%"

echo Compilando...
rem -static and -static-libgcc/-static-libstdc++ bake the C++ runtime into the .exe,
rem so it runs on PCs without MinGW DLLs.
g++ -std=c++17 -O2 -Wall -Wextra -static -static-libgcc -static-libstdc++ -Isrc -I"%RAYLIB_DIR%" src\main.cpp src\terminal\terminal_view.cpp src\audio\music_player.cpp src\narrative\json.cpp src\narrative\content.cpp src\simulation\simulation.cpp src\systems\condition.cpp src\systems\time_system.cpp src\systems\event_system.cpp src\systems\ending_system.cpp src\systems\evolution_system.cpp src\terminal\text_wrap.cpp src\terminal\actions.cpp src\terminal\command_parser.cpp src\save\save_system.cpp -o "%OUT%\please_dont_die.exe" -L"%RAYLIB_DIR%" -lraylib -lopengl32 -lgdi32 -lwinmm
if errorlevel 1 goto failed

xcopy /E /I /Y /Q data "%OUT%\data" >nul
xcopy /E /I /Y /Q audio "%OUT%\audio" >nul
if not exist "%OUT%\saves" mkdir "%OUT%\saves"

echo.
echo Compilacion correcta: %OUT%\please_dont_die.exe

if /I "%~1"=="run" goto run_game
if /I "%~1"=="dist" goto make_dist
goto end

:run_game
pushd "%OUT%"
"please_dont_die.exe"
popd
goto end

:make_dist
if exist "%DIST%" rmdir /S /Q "%DIST%"
mkdir "%DIST%"
copy /Y "%OUT%\please_dont_die.exe" "%DIST%\" >nul
xcopy /E /I /Y /Q data "%DIST%\data" >nul
xcopy /E /I /Y /Q audio "%DIST%\audio" >nul
mkdir "%DIST%\saves"
if exist "dist\PleaseDontDie-v0.1.zip" del /Q "dist\PleaseDontDie-v0.1.zip"
powershell -NoProfile -Command "Compress-Archive -Path 'dist\PleaseDontDie' -DestinationPath 'dist\PleaseDontDie-v0.1.zip'"
if errorlevel 1 goto failed
echo.
echo Paquete listo: dist\PleaseDontDie-v0.1.zip
goto end

:no_header
echo [ERROR] No encuentro raylib.h en "%RAYLIB_DIR%".
exit /b 1

:no_lib
echo [ERROR] No encuentro libraylib.a en "%RAYLIB_DIR%". Compilala con mingw32-make.
exit /b 1

:failed
echo.
echo [ERROR] La compilacion fallo. Revisa los mensajes de arriba.
exit /b 1

:end
endlocal
