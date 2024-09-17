//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include <string>

namespace GoogleDrive
{
    bool Ready();
    bool HasToken();
    /// @param aTokenCallback The name of a javascript accessible function which takes an Emval float containing the expiration time for the token.
    void RequestToken(bool aShowPopup, std::string aTokenCallback);
    void LogOut();

    /// @brief Shows a Google Drive file picker.
    /// @param someMimeTypes Sets the types of files the user can choose from.
    /// @param aFileCallbackName The name of a javascript accessible function which takes an Emval string containing the fs path and the gd file-id that was loaded. Might be called multiple times.
    /// @param aCancelCallbackName The name of a javascript accessible function that is called if the user pressed 'Cancel'.
    void LoadProject(std::string someMimeTypes, std::string aFileCallbackName, std::string aDoneCallbackName, std::string aCancelCallbackName);
    void SaveProject(std::string aFileID, std::string aFilePath);
}