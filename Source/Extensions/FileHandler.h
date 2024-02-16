#include <string>

namespace FileHandler
{
    std::string OpenFolder(const char* aMode = "read");
    void DownloadDocument(const char* aPath);
}