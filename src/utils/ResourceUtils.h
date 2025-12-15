#pragma once

#include <string>

class ResourceUtils {
public:
    // Returns the absolute path to a resource file.
    // e.g., getPath("images/background.png") -> "C:/.../resources/images/background.png"
    static std::string getPath(const std::string& relativePath);
    
    // Returns the absolute path to the resources directory
    static std::string getResourcesDir();
};
