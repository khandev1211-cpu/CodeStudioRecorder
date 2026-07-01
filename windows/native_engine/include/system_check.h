#pragma once
#include <string>
#include <vector>

namespace cs {

struct SystemRequirement {
    std::string name;
    bool met;
    std::string details;
};

class SystemCheck {
public:
    static std::vector<SystemRequirement> runFullCheck();

    static bool checkFFmpeg();
    static bool checkD3D11();
    static bool checkWinVersion();
    static bool checkPermissions();
};

} // namespace cs
