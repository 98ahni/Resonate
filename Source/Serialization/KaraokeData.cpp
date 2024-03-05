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
    KaraokeLine &KaraokeDocument::GetValidLineAfter(size_t aLine)
    {
        // TODO: insert return statement here
        return myTokens[aLine];
    }
    KaraokeLine &KaraokeDocument::GetValidLineBefore(size_t aLine)
    {
        // TODO: insert return statement here
        return myTokens[aLine];
    }
    KaraokeToken &KaraokeDocument::GetToken(size_t aLine, size_t aToken)
    {
        return myTokens[aLine][aToken];
    }
    KaraokeToken &KaraokeDocument::GetTokenAfter(size_t aLine, size_t aToken)
    {
        if(aToken + 1 < myTokens[aLine].size())
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
    KaraokeToken &KaraokeDocument::GetTokenBefore(size_t aLine, size_t aToken)
    {
        if(aToken - 1 >= 0)
        {
            return myTokens[aLine][aToken - 1];
        }
        else if(aLine - 1 >= 0)
        {
            if(myTokens[aLine - 1].size() > 0)
            {
                return myTokens[aLine - 1][myTokens[aLine - 1].size() - 1];
            }
            else
            {
                return GetTokenBefore(aLine - 1, 0);
            }
        }
        return ourNullToken;
    }
    KaraokeToken &KaraokeDocument::GetTimedTokenAfter(size_t aLine, size_t aToken)
    {
        // TODO: insert return statement here
        return ourNullToken;
    }
    KaraokeToken &KaraokeDocument::GetTimedTokenBefore(size_t aLine, size_t aToken)
    {
        // TODO: insert return statement here
        return ourNullToken;
    }
    uint KaraokeDocument::GetBaseStartColor()
    {
        return myBaseStartColor;
    }
    uint KaraokeDocument::GetBaseEndColor()
    {
        return myBaseEndColor;
    }
    void KaraokeDocument::Clear()
    {
        for(int i = 0; i < myTokens.size(); i++)
        {
            myTokens[i].clear();
        }
        myTokens.clear();
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
        Clear();
        myPath = aPath;
        std::ifstream docFile(aPath);
        std::string line;
        while(std::getline(docFile, line))
        {
            ParseLine(line);
        }
        docFile.close();
    }
    void KaraokeDocument::Parse(std::string aDocument)
    {
        Clear();
        std::vector<std::string> lines = StringTools::Split(aDocument, "\n");
        for(int i = 0; i < lines.size(); i++)
        {
            ParseLine(lines[i]);
        }
    }
    void KaraokeDocument::ParseLine(std::string aLine)
    {
        if(aLine.starts_with("start color"))
        {

            return;
        }
        if(aLine.starts_with("end color"))
        {
            return;
        }
            myTokens.push_back(std::vector<KaraokeToken>());
            std::vector<std::string> rawTokens = StringTools::Split(aLine, std::regex("(\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\])?(.(?!(\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\])))*.?"), true);
            for(int i = 0; i < rawTokens.size(); i++)
            {
                std::vector<std::string> timeStamp = StringTools::Split(rawTokens[i], std::regex("\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\]"), true);
                std::vector<std::string> token = StringTools::Split(rawTokens[i], std::regex("\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\]"), false);
                if(timeStamp.size())
                {
                    for(int t = 0; t < timeStamp.size(); t++)
                    {
                        myTokens.back().push_back({token[t + 1].size() ? token[t + 1] : "", true, StringToTime(timeStamp[t])});
                    }
                }
                else if(token.size() && token[0] != "")
                {
                    myTokens.back().push_back({token[0], false, 0});
                }
                //if(token.size()) printf("%i:%s", token[0].size(), token[0].c_str());
                //for(int j = 1; j < token.size(); j++)
                //    printf(", %i:%s", token[j].size(), token[j].c_str());
                //printf("\n");
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
    std::string KaraokeDocument::Save()
    {
        std::ofstream docFile(myPath);
        docFile.clear();
        docFile << Serialize();
        return myPath;
    }
    uint KaraokeDocument::StringToTime(std::string aTimeStr)
    {
        //printf("[%s:%s:%s]\n", aTimeStr.substr(1, 2).c_str(), aTimeStr.substr(4, 2).c_str(), aTimeStr.substr(7, 2).c_str());
        return
        (std::stoi(aTimeStr.substr(1, 2)) * 60 * 100) +
        (std::stoi(aTimeStr.substr(4, 2)) * 100) +
        (std::stoi(aTimeStr.substr(7, 2)));
    }
    std::string KaraokeDocument::TimeToString(uint aTime)
    {
        return (std::stringstream() << "[" << std::setfill('0') << std::setw(2) << (aTime / 100) / 60 << ":" << std::setw(2) << (aTime / 100) % 60 << ":" << std::setw(2) << (aTime % 100) << "]").str();
    }
    uint KaraokeDocument::FromHex(std::string someHex)
    {
        uint output = 0;
        for(int i = 0; i < someHex.size(); i++)
        {
            
        }
        return;
    }
    std::string KaraokeDocument::ToHex(uint aNum)
    {
        return std::string();
    }
    bool KaraokeDocument::IsNull(KaraokeToken &aToken)
    {
        return
        aToken.myHasStart == ourNullToken.myHasStart &&
        aToken.myValue == ourNullToken.myValue;
    }
    bool KaraokeDocument::IsNull(KaraokeLine &aLine)
    {
        return aLine.size() == 0;
    }
}