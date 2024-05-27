#include <string>

namespace GoogleDrive
{
    bool Ready();
    bool HasToken();
    void RequestToken();
    void LogOut();

    /// @brief Shows a Google Drive file picker.
    /// @param someMimeTypes Sets the types of files the user can choose from.
    /// @param aCallbackName The name of a javascript accessible function which takes a string containing the fs path that was loaded.
    void LoadProject(std::string someMimeTypes, std::string aCallbackName);
    std::string SaveProject();
}