#pragma once

#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Platform-aware path utilities for Blender detection
// ─────────────────────────────────────────────────────────────────────────────

namespace PlatformPaths {

/// Get all common default Blender installation paths for the current platform
std::vector<std::string> getDefaultBlenderPaths();

/// Get the default projects path for the current platform
std::string getDefaultProjectsPath();

}  // namespace PlatformPaths
