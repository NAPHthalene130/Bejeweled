#include "ResourceUtils.h"
#include <iostream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

std::string ResourceUtils::getResourcesDir() {
    // 1. Check PROJECT_SOURCE_DIR (passed from CMake)
    // This is the most reliable method for development environment
#ifdef PROJECT_SOURCE_DIR
    // PROJECT_SOURCE_DIR is expected to be a string literal, e.g., "C:/Path/To/Project"
    fs::path sourceDir(PROJECT_SOURCE_DIR);
    fs::path resourceDir = sourceDir / "resources";
    if (fs::exists(resourceDir)) {
        return resourceDir.string();
    }
#endif

    // 2. Fallback: Search relative to the executable/working directory
    // This handles cases where the executable is in a build folder (e.g., build/Debug)
    // or when deployed without the source code (if resources are copied relative to exe)
    fs::path currentPath = fs::current_path();
    
    // Check up to 3 levels up
    std::vector<fs::path> candidates;
    candidates.push_back(currentPath / "resources");
    candidates.push_back(currentPath.parent_path() / "resources");
    candidates.push_back(currentPath.parent_path().parent_path() / "resources");
    candidates.push_back(currentPath.parent_path().parent_path().parent_path() / "resources");

    for (const auto& path : candidates) {
        // We use exists() and is_directory() to ensure it's valid
        std::error_code ec;
        if (fs::exists(path, ec) && fs::is_directory(path, ec)) {
            return fs::absolute(path).string();
        }
    }

    // Not found
    std::cerr << "Warning: 'resources' directory not found in project source or parent directories." << std::endl;
    return "";
}

std::string ResourceUtils::getPath(const std::string& relativePath) {
    std::string base = getResourcesDir();
    if (base.empty()) {
        return relativePath; // Just return what was asked if we can't find the base
    }
    
    fs::path fullPath = fs::path(base) / relativePath;
    return fullPath.string();
}
