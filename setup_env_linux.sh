#!/usr/bin/env bash
# =============================================================================
# setup_env_linux.sh  -  Installation des dependances pour Blendgment
#
# Dependances requises :
#   - Compilateur C++17  (g++ ou clang)
#   - CMake >= 3.20 + Ninja
#   - Vulkan SDK  (loader + headers + validation layers)
#   - GLFW 3.3+
#   - libcurl
#   - libarchive
#
# ImGUI est telecharge automatiquement par CMake (FetchContent).
# =============================================================================

set -euo pipefail

GREEN="\033[0;32m"
CYAN="\033[0;36m"
RED="\033[0;31m"
NC="\033[0m"

info()  { printf "${CYAN}[INFO]${NC}  %s\n" "$1"; }
ok()    { printf "${GREEN}[ OK ]${NC}  %s\n" "$1"; }
err()   { printf "${RED}[ERR ]${NC}  %s\n" "$1"; exit 1; }

# -- Detection du gestionnaire de paquets ------------------------------------
if   command -v apt-get &>/dev/null; then PM="apt"
elif command -v dnf     &>/dev/null; then PM="dnf"
elif command -v pacman  &>/dev/null; then PM="pacman"
else err "Gestionnaire de paquets non supporte. Installez manuellement les dependances."
fi

info "Systeme detecte : $PM"

# -- Installation -------------------------------------------------------------
case $PM in

  apt)
    sudo apt-get update -qq
    sudo apt-get install -y \
        build-essential \
        cmake \
        ninja-build \
        libvulkan-dev \
        vulkan-tools \
        vulkan-validationlayers-dev \
        glslang-tools \
        libglfw3-dev \
        libcurl4-openssl-dev \
        libarchive-dev
    ;;

  dnf)
    sudo dnf install -y \
        gcc-c++ \
        cmake \
        ninja-build \
        vulkan-loader-devel \
        vulkan-tools \
        vulkan-validation-layers-devel \
        glslang \
        glfw-devel \
        libcurl-devel \
        libarchive-devel
    ;;

  pacman)
    sudo pacman -Sy --needed \
        base-devel \
        cmake \
        ninja \
        vulkan-headers \
        vulkan-icd-loader \
        vulkan-tools \
        vulkan-validation-layers \
        glslang \
        glfw \
        curl \
        libarchive
    ;;

esac

ok "Dependances installees."

# -- Vulkan SDK complet (optionnel) -------------------------------------------
echo ""
info "Les paquets systeme suffisent pour compiler et executer Blendgment."
info "SDK LunarG complet (optionnel) : https://vulkan.lunarg.com/sdk/home#linux"
echo ""

# -- Build --------------------------------------------------------------------
ok "Environnement pret. Pour compiler :"
echo "  cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release"
echo "  cmake --build build"
