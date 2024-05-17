#include <string>

namespace FileHandler
{
    std::string OpenFolder(const char* aMode = "read");
    void DownloadDocument(const char* aPath);
    void SetLocalValue(std::string aName, std::string aValue);
    std::string GetLocalValue(std::string aName);
    void RemoveLocalValue(std::string aName);
    void ClearLocalStorage();
}