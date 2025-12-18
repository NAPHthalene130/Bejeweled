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
    // 如果没有 resources，优先使用项目内的 assets 目录（便于开发环境）
    fs::path assetsDir = sourceDir / "assets";
    if (fs::exists(assetsDir) && fs::is_directory(assetsDir)) {
        return assetsDir.string();
    }
#endif

    // 2. Fallback: Search relative to the executable/working directory
    // This handles cases where the executable is in a build folder (e.g., build/Debug)
    // or when deployed without the source code (if resources are copied relative to exe)
    fs::path currentPath = fs::current_path();
    
    // Check up to 3 levels up
    std::vector<fs::path> candidates;
    // 在可执行目录及上级目录查找 resources 或 assets
    candidates.push_back(currentPath / "resources");
    candidates.push_back(currentPath / "assets");
    candidates.push_back(currentPath.parent_path() / "resources");
    candidates.push_back(currentPath.parent_path() / "assets");
    candidates.push_back(currentPath.parent_path().parent_path() / "resources");
    candidates.push_back(currentPath.parent_path().parent_path() / "assets");
    candidates.push_back(currentPath.parent_path().parent_path().parent_path() / "resources");
    candidates.push_back(currentPath.parent_path().parent_path().parent_path() / "assets");

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
    fs::path inputPath(relativePath);
    if (inputPath.is_absolute()) {
        if (inputPath.filename() == "user_bg.png") {
            return (inputPath.parent_path() / "achievement_bg.png").string();
        }
        if (inputPath.filename() == "default_bg.png") {
            return (inputPath.parent_path() / "final_bg.png").string();
        }
        return relativePath;
    }

    std::string normalized = relativePath;
    if (normalized == "images/user_bg.png") {
        normalized = "images/achievement_bg.png";
    } else if (normalized == "images/default_bg.png") {
        normalized = "images/final_bg.png";
    }

    std::string base = getResourcesDir();
    if (base.empty()) {
        return normalized; // Just return what was asked if we can't find the base
    }
    
    fs::path fullPath = fs::path(base) / normalized;
    return fullPath.string();
}
