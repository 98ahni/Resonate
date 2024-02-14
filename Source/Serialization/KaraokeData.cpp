#include "KaraokeData.h"
#include <filesystem>
#include <fstream>
#include <StringTools.h>
#include <sstream>
#include <iomanip>

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
    KaraokeLine& KaraokeDocument::GetLine(size_t aLine)
    {
        return myTokens[aLine];
    }
    KaraokeToken &KaraokeDocument::GetToken(size_t aLine, size_t aToken)
    {
        return myTokens[aLine][aToken];
    }
    KaraokeToken &KaraokeDocument::GetTokenAfter(size_t aLine, size_t aToken)
    {
        if(myTokens[aLine].size() < aToken + 1)
        {
            return myTokens[aLine][aToken + 1];
        }
        else if(aLine + 1 < myTokens.size())
        {
            if(myTokens[aLine + 1].size() > 0)
            {
                return myTokens[aLine + 1][0];
            }
            else
            {
                return GetTokenAfter(aLine + 1, 0);
            }
        }
        return ourNullToken;
    }
    void KaraokeDocument::Load(std::string aPath)
    {
        printf("Loading %s.\n", aPath.c_str());
        if(!std::filesystem::exists(aPath))
        {
            printf("%s does not exist!\n", aPath.c_str());
        }
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
            printf("No text document found!\n");
            return;
        }
        std::ifstream docFile(aPath);
        std::string line;
        while(std::getline(docFile, line))
        {
            ParseLine(line);
        }
    }
    void KaraokeDocument::Parse(std::string aDocument)
    {
        std::vector<std::string> lines = StringTools::Split(aDocument, "\n");
        for(int i = 0; i < lines.size(); i++)
        {
            ParseLine(lines[i]);
        }
    }
    void KaraokeDocument::ParseLine(std::string aLine)
    {
            myTokens.push_back(std::vector<KaraokeToken>());
            std::vector<std::string> rawTokens = StringTools::Split(aLine, std::regex("\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\](.(?!(\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\])))*."), true);
            for(int i = 0; i < rawTokens.size(); i++)
            {
                std::vector<std::string> timeStamp = StringTools::Split(rawTokens[i], std::regex("\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\]"), true);
                std::vector<std::string> token = StringTools::Split(rawTokens[i], std::regex("\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\]"), false);
                myTokens.back().push_back({token.size() > 1 ? token[1] : "", !timeStamp.empty(), timeStamp.empty() ? 0 : StringToTime(timeStamp[0])});
                if(token.size()) printf("%s", token[0].c_str());
                for(int j = 1; j < token.size(); j++)
                    printf(", %s", token[j].c_str());
                printf("\n");
            }
    }
    std::string KaraokeDocument::Serialize()
    {
        std::string output;
        for(int line = 0; line < myTokens.size(); line++)
        {
            for(int token = 0; token < myTokens[line].size(); token++)
            {
                output += (myTokens[line][token].myHasStart ? TimeToString(myTokens[line][token].myStartTime) : "") + myTokens[line][token].myValue;
            }
            output += "\n";
        }
        return output;
    }
    uint KaraokeDocument::StringToTime(std::string aTimeStr)
    {
        printf("[%s:%s:%s]\n", aTimeStr.substr(1, 2).c_str(), aTimeStr.substr(4, 2).c_str(), aTimeStr.substr(7, 2).c_str());
        return
        (std::stoi(aTimeStr.substr(1, 2)) * 60 * 100) +
        (std::stoi(aTimeStr.substr(4, 2)) * 100) +
        (std::stoi(aTimeStr.substr(7, 2)));
    }
    std::string KaraokeDocument::TimeToString(uint aTime)
    {
        return (std::stringstream() << "[" << std::setfill('0') << std::setw(2) << (aTime / 100) / 60 << ":" << std::setw(2) << (aTime / 100) % 60 << ":" << std::setw(2) << (aTime % 100) << "]").str();
    }
}