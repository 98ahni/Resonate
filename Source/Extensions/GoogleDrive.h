#include <string>

namespace GoogleDrive
{
    bool Ready();
    bool HasToken();
    void RequestToken();
    void LogOut();

    /// @brief Shows a Google Drive file picker.
    /// @param someMimeTypes Sets the types of files the user can choose from.
    /// @param aFileCallbackName The name of a javascript accessible function which takes an Emval string containing the fs path and the gd file-id that was loaded. Might be called multiple times.
    /// @param aCancelCallbackName The name of a javascript accessible function that is called if the user pressed 'Cancel'.
    void LoadProject(std::string someMimeTypes, std::string aFileCallbackName, std::string aCancelCallbackName);
    std::string SaveProject();
}