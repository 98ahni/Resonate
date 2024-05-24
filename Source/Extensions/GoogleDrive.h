#include <string>

namespace GoogleDrive
{
    bool Ready();
    bool HasToken();
    void RequestToken();
    void LogOut();

    std::string LoadProject();
    std::string SaveProject();
}