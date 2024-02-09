#include "KaraokeData.h"
#include <filesystem>
#include <fstream>

namespace Serialization
{
    KaraokeDocument* KaraokeDocument::ourInstance = new KaraokeDocument();
    KaraokeDocument& KaraokeDocument::Get()
    {
        return *ourInstance;
    }
    KaraokeData& KaraokeDocument::GetData()
    {
        return myTokens;
    }
    KaraokeLine& KaraokeDocument::GetLine(size_t i)
    {
        return myTokens[i];
    }
    void KaraokeDocument::Load(std::string aPath)
    {
        if(std::filesystem::is_directory(aPath))
        {
            for (auto &path : std::filesystem::directory_iterator(aPath))
            {
                if (path.path().extension() == ".txt")
                {
                    Load(path.path().string());
                    return;
                }
            }
            return;
        }
        std::ifstream docFile(aPath);
        std::string line;
        while(std::getline(docFile, line))
        {
            myTokens.push_back(std::vector<KaraokeToken>());
        }
    }
}