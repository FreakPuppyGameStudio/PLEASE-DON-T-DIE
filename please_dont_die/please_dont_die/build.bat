@echo off
rem ============================================================
rem  PLEASE, DON'T DIE v0.1 - build without CMake
rem  Needs: Visual Studio (C++ workload) and the Raylib MSVC zip.
rem  Usage:  build.bat          -> compiles to build_bat\
rem          build.bat run      -> compiles and runs the game
rem          build.bat test     -> compiles and runs the logic tests
rem ============================================================
setlocal
cd /d "%~dp0"

rem ---- EDIT THESE: folders from the Raylib MSVC zip -------------
rem RAYLIB_INC must contain raylib.h
rem RAYLIB_LIB must contain raylib.lib
set "RAYLIB_INC=C:\raylib\include"
set "RAYLIB_LIB=C:\raylib\lib"
rem ----------------------------------------------------------------

set "OUT=build_bat"

if not exist "%RAYLIB_INC%\raylib.h" goto no_raylib_h
if not exist "%RAYLIB_LIB%\raylib.lib" goto no_raylib_lib

rem Load the MSVC compiler if cl.exe is not already available.
where cl >nul 2>nul
if not errorlevel 1 goto have_cl

set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARS%" goto load_vcvars
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARS%" goto load_vcvars
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARS%" goto load_vcvars
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
if exist "%VCVARS%" goto load_vcvars
goto no_cl

:load_vcvars
call "%VCVARS%" x64 >nul
where cl >nul 2>nul
if errorlevel 1 goto no_cl

:have_cl
if not exist "%OUT%" mkdir "%OUT%"

set "SRC=src\main.cpp src\terminal\terminal_view.cpp src\audio\music_player.cpp src\narrative\json.cpp src\narrative\content.cpp src\simulation\simulation.cpp src\systems\condition.cpp src\systems\time_system.cpp src\systems\event_system.cpp src\systems\ending_system.cpp src\systems\evolution_system.cpp src\terminal\text_wrap.cpp src\terminal\actions.cpp src\terminal\command_parser.cpp src\save\save_system.cpp"

echo Compiling the game...
cl /nologo /std:c++17 /EHsc /W3 /O2 /I src /I "%RAYLIB_INC%" %SRC% /Fe"%OUT%\please_dont_die.exe" /Fo"%OUT%\\" /link /LIBPATH:"%RAYLIB_LIB%" raylib.lib winmm.lib gdi32.lib user32.lib shell32.lib opengl32.lib
if errorlevel 1 goto build_failed

rem Copy the game data next to the executable.
xcopy /E /I /Y /Q data "%OUT%\data" >nul
xcopy /E /I /Y /Q audio "%OUT%\audio" >nul
if not exist "%OUT%\saves" mkdir "%OUT%\saves"

echo.
echo Build OK: %OUT%\please_dont_die.exe

if /I "%~1"=="test" goto run_tests
if /I "%~1"=="run" goto run_game
goto end

:run_game
pushd "%OUT%"
"please_dont_die.exe"
popd
goto end

:run_tests
echo Compiling the logic tests...
cl /nologo /std:c++17 /EHsc /W3 /I src tests\test_logic.cpp src\narrative\json.cpp src\narrative\content.cpp src\simulation\simulation.cpp src\systems\condition.cpp src\systems\time_system.cpp src\systems\event_system.cpp src\systems\ending_system.cpp src\systems\evolution_system.cpp src\terminal\text_wrap.cpp src\terminal\actions.cpp src\terminal\command_parser.cpp src\save\save_system.cpp /Fe"%OUT%\test_logic.exe" /Fo"%OUT%\\" >nul
if errorlevel 1 goto build_failed
pushd "%OUT%"
"test_logic.exe"
popd
goto end

:no_raylib_h
echo [ERROR] raylib.h not found in "%RAYLIB_INC%".
echo Edit RAYLIB_INC at the top of build.bat.
exit /b 1

:no_raylib_lib
echo [ERROR] raylib.lib not found in "%RAYLIB_LIB%".
echo Edit RAYLIB_LIB at the top of build.bat.
exit /b 1

:no_cl
echo [ERROR] Could not find the Visual Studio C++ compiler (cl.exe).
echo Install Visual Studio 2022 with the "Desarrollo para escritorio con C++" workload,
echo or open "Developer Command Prompt for VS 2022" and run build.bat from there.
exit /b 1

:build_failed
echo.
echo [ERROR] Build failed. Read the messages above.
exit /b 1

:end
endlocal
