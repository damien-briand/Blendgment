@echo off
setlocal EnableDelayedExpansion

rem =============================================================================
rem setup_env_windows.bat  -  Installation des dependances pour Blendgment
rem
rem Dependances requises :
rem   - Visual Studio 2022 (ou Build Tools) avec workload "C++ Desktop"
rem   - CMake >= 3.20
rem   - Ninja
rem   - Vulkan SDK LunarG
rem   - vcpkg  (pour GLFW, libcurl, libarchive)
rem
rem ImGUI est telecharge automatiquement par CMake (FetchContent).
rem =============================================================================

echo.
echo ============================================================
echo  Blendgment - Setup environnement Windows
echo ============================================================
echo.

rem -- 1. Verifier CMake -------------------------------------------------------
cmake --version >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERREUR] CMake non trouve.
    echo Telechargez-le depuis https://cmake.org/download/
    echo Cochez "Add CMake to system PATH" lors de l'installation.
    pause & exit /b 1
)
echo [ OK ] CMake detecte.

rem -- 2. Verifier Ninja -------------------------------------------------------
ninja --version >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [INFO] Ninja non trouve. Installation via winget...
    winget install --id Ninja-build.Ninja -e --silent
    if %ERRORLEVEL% neq 0 (
        echo [ERREUR] Impossible d'installer Ninja automatiquement.
        echo Telechargez-le depuis https://ninja-build.org/ et ajoutez-le au PATH.
        pause & exit /b 1
    )
)
echo [ OK ] Ninja detecte.

rem -- 3. Verifier le Vulkan SDK -----------------------------------------------
if not defined VULKAN_SDK (
    echo.
    echo [ERREUR] Vulkan SDK non detecte ^(variable VULKAN_SDK absente^).
    echo Telechargez et installez le SDK depuis :
    echo   https://vulkan.lunarg.com/sdk/home#windows
    echo Relancez ce script apres l'installation.
    pause & exit /b 1
)
echo [ OK ] Vulkan SDK : %VULKAN_SDK%

rem -- 4. Verifier vcpkg -------------------------------------------------------
vcpkg version >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo.
    echo [INFO] vcpkg non trouve dans le PATH.
    echo Installez vcpkg :
    echo   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
    echo   C:\vcpkg\bootstrap-vcpkg.bat
    echo   Ajoutez C:\vcpkg au PATH puis relancez ce script.
    pause & exit /b 1
)
echo [ OK ] vcpkg detecte.

rem -- 5. Installer les dependances via vcpkg ----------------------------------
echo.
echo [INFO] Installation des dependances via vcpkg (x64-windows)...

vcpkg install glfw3:x64-windows
if %ERRORLEVEL% neq 0 ( echo [ERREUR] Echec glfw3 & pause & exit /b 1 )
echo [ OK ] glfw3

vcpkg install curl:x64-windows
if %ERRORLEVEL% neq 0 ( echo [ERREUR] Echec curl & pause & exit /b 1 )
echo [ OK ] curl

vcpkg install libarchive:x64-windows
if %ERRORLEVEL% neq 0 ( echo [ERREUR] Echec libarchive & pause & exit /b 1 )
echo [ OK ] libarchive

rem -- 6. Recap build ----------------------------------------------------------
echo.
echo ============================================================
echo  Environnement pret !
echo ============================================================
echo.
echo Pour compiler le projet, executez dans ce dossier :
echo.
echo   cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release ^
echo         -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
echo   cmake --build build
echo.

pause
endlocal
