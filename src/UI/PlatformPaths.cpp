#include "PlatformPaths.h"
#include <filesystem>
#include <sstream>
#include <cstdlib>

namespace fs = std::filesystem;

namespace PlatformPaths {

std::vector<std::string> getDefaultBlenderPaths()
{
    std::vector<std::string> paths;

#ifdef _WIN32
    // Windows: Check common installation directories
    const char* programFiles32 = std::getenv("ProgramFiles(x86)");
    const char* programFiles64 = std::getenv("ProgramFiles");
    const char* appData = std::getenv("APPDATA");
    const char* userProfile = std::getenv("USERPROFILE");

    // Program Files (x86) and (x64) - typical installation location
    if (programFiles64) {
        paths.push_back(std::string(programFiles64) + "\\Blender");
    }
    if (programFiles32) {
        paths.push_back(std::string(programFiles32) + "\\Blender");
    }

    // AppData/Local/BlenderFoundation - portable or user installations
    if (appData) {
        paths.push_back(std::string(appData) + "\\..\\Local\\BlenderFoundation\\Blender");
    }

    // User home directory - custom installations
    if (userProfile) {
        paths.push_back(std::string(userProfile) + "\\Blender");
        paths.push_back(std::string(userProfile) + "\\Documents\\Blender");
        paths.push_back(std::string(userProfile) + "\\Downloads\\Blender");
    }

    // D:\Blender is also common for extra disk space
    if (fs::exists("D:\\Blender")) {
        paths.push_back("D:\\Blender");
    }

#elif __APPLE__
    // macOS: Standard installation locations
    paths.push_back("/Applications/Blender.app/Contents/Resources");
    paths.push_back("/opt/Blender");
    
    const char* home = std::getenv("HOME");
    if (home) {
        paths.push_back(std::string(home) + "/Blender");
        paths.push_back(std::string(home) + "/Applications/Blender.app/Contents/Resources");
    }

#else
    // Linux: Standard and custom paths
    paths.push_back("/opt/blender");
    paths.push_back("/usr/local/blender");
    paths.push_back("/home");  // Generic, users may install in home
    
    const char* home = std::getenv("HOME");
    if (home) {
        paths.push_back(std::string(home) + "/blender");
        paths.push_back(std::string(home) + "/Downloads");  // Check Downloads
    }
#endif

    // Filter out non-existent paths
    std::vector<std::string> existing;
    std::error_code ec;
    for (const auto& p : paths) {
        if (fs::exists(p, ec)) {
            existing.push_back(p);
        }
    }

    return existing;
}

std::string getDefaultProjectsPath()
{
    std::string path;

#ifdef _WIN32
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile) {
        path = std::string(userProfile) + "\\Documents\\Blendgment\\projects";
    } else {
        path = ".\\projects";
    }
#else
    const char* home = std::getenv("HOME");
    if (home) {
        path = std::string(home) + "/Blendgment/projects";
    } else {
        path = "./projects";
    }
#endif

    return path;
}

}  // namespace PlatformPaths
