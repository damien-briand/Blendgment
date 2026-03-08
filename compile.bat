@echo off
setlocal EnableDelayedExpansion

rem =============================================================================
rem compile.bat  -  Script de compilation Blendgment pour Windows
rem Usage : compile.bat [release|debug] [clean]
rem =============================================================================

set BUILD_TYPE_RAW=%1
set CLEAN_FLAG=%2

if "%BUILD_TYPE_RAW%"=="" set BUILD_TYPE_RAW=release
if "%BUILD_TYPE_RAW%"=="clean" ( set CLEAN_FLAG=clean & set BUILD_TYPE_RAW=release )

rem -- Type de build -----------------------------------------------------------
if /i "%BUILD_TYPE_RAW%"=="release" ( set BUILD_TYPE=Release )
if /i "%BUILD_TYPE_RAW%"=="debug"   ( set BUILD_TYPE=Debug )
if not defined BUILD_TYPE (
    echo [ERREUR] Type de build invalide : %BUILD_TYPE_RAW%
    echo Usage : compile.bat [release^|debug] [clean]
    exit /b 1
)

rem -- Toolchain vcpkg ---------------------------------------------------------
if defined VCPKG_INSTALLATION_ROOT (
    set VCPKG_TOOLCHAIN=%VCPKG_INSTALLATION_ROOT%\scripts\buildsystems\vcpkg.cmake
) else if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set VCPKG_TOOLCHAIN=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
) else (
    echo [ERREUR] vcpkg introuvable. Lancez setup_env_windows.bat d'abord.
    exit /b 1
)

rem -- Nettoyage ---------------------------------------------------------------
if /i "%CLEAN_FLAG%"=="clean" (
    echo [INFO] Nettoyage du dossier build...
    if exist build rmdir /s /q build
    echo [ OK ] Build nettoye.
)

rem -- Configure ---------------------------------------------------------------
echo [INFO] Configuration CMake (%BUILD_TYPE%)...
cmake -B build -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN%"
if %ERRORLEVEL% neq 0 ( echo [ERREUR] Configuration echouee. & exit /b 1 )

rem -- Compilation -------------------------------------------------------------
echo [INFO] Compilation...
cmake --build build --parallel
if %ERRORLEVEL% neq 0 ( echo [ERREUR] Compilation echouee. & exit /b 1 )

echo.
echo [ OK ] Blendgment compile avec succes en %BUILD_TYPE% !
echo        Executable : build\Blendgment.exe
endlocal
