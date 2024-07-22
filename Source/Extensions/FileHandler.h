//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include <string>

namespace FileHandler
{
    std::string OpenFolder(const char* aMode = "read");
    std::string OpenDocument(const char* aSaveFolder = ".", const char* aFileType = "*", const char* aMode = "read");
    void DownloadDocument(const char* aPath);
    void SyncLocalFS();
    void SetLocalValue(std::string aName, std::string aValue);
    std::string GetLocalValue(std::string aName);
    void RemoveLocalValue(std::string aName);
    void ClearLocalStorage();
}