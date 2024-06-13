//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "KaraokeData.h"
#include <emscripten.h>
#include <filesystem>
#include <fstream>
#include <StringTools.h>
#include <Extensions/FileHandler.h>
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
        if(aLine + 1 >= myTokens.size())
        {
            return ourNullLine;
        }
        if(myTokens[aLine + 1].size() == 0)
        {
            return GetValidLineAfter(aLine + 1);
        }
        return myTokens[aLine + 1];
    }
    KaraokeLine &KaraokeDocument::GetValidLineBefore(size_t aLine)
    {
        if(aLine <= 0)
        {
            return ourNullLine;
        }
        if(myTokens[aLine - 1].size() == 0)
        {
            return GetValidLineBefore(aLine - 1);
        }
        return myTokens[aLine - 1];
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
        if(aToken > 0)
        {
            return GetLine(aLine)[aToken - 1];
        }
        else if(aLine > 0)
        {
            //if(GetLine(aLine - 1).size() > 0)
            //{
            //    return GetLine(aLine - 1).back();
            //}
            //else
            //{
            //    return GetTokenBefore(aLine - 1, 0);
            //}
            return GetValidLineBefore(aLine).back();
        }
        return ourNullToken;
    }
    KaraokeToken &KaraokeDocument::GetTimedTokenAfter(size_t aLine, size_t aToken)
    {
        KaraokeToken& token = GetTokenAfter(aLine, aToken);
        if(IsNull(token))
        {
            return ourNullToken;
        }
        if(token.myHasStart)
        {
            return token;
        }
        if(aToken + 1 < myTokens[aLine].size())
        {
            return GetTimedTokenAfter(aLine, aToken + 1);
        }
        return GetTimedTokenAfter(aLine + 1, 0);
    }
    KaraokeToken &KaraokeDocument::GetTimedTokenBefore(size_t aLine, size_t aToken)
    {
        KaraokeToken& token = GetTokenBefore(aLine, aToken);
        if(IsNull(token))
        {
            return ourNullToken;
        }
        if(token.myHasStart)
        {
            return token;
        }
        if(aToken > 0)
        {
            return GetTimedTokenBefore(aLine, aToken - 1);
        }
        return GetTimedTokenBefore(aLine - 1, 0);
    }
    bool KaraokeDocument::IsPauseToken(size_t aLine, size_t aToken)
    {
        return IsPauseToken(GetToken(aLine, aToken));
    }
    bool KaraokeDocument::IsPauseToken(KaraokeToken& aToken)
    {
        return (aToken.myValue.empty() || aToken.myValue == " ") && !IsNull(aToken);
    }
    uint KaraokeDocument::GetStartColor()
    {
        return myHasOverrideColor ? myOverrideStartColor : myBaseStartColor;
    }
    uint KaraokeDocument::GetEndColor()
    {
        return myHasOverrideColor ? myOverrideEndColor : myBaseEndColor;
    }
    bool KaraokeDocument::ParseEffectToken(KaraokeToken &aToken)
    {
        std::vector<std::string> tags = StringTools::Split(aToken.myValue.data(), std::regex("<[A-Za-z0-9#\"= ]+>"), true);
        if(tags.size() == 0)
        {
            return false;
        }
        bool output = false;
        for(int i = 0; i < tags.size(); i++)
        {
            if(i >= tags.size()) {printf("The for loop broke all logic and found \"%i\" to be less than \"%i\"!\n", i, tags.size()); break;}
            if(tags[i].empty()) continue;
            std::string possibleAlias = tags[i];
            StringTools::EraseSubString(possibleAlias, "<");
            StringTools::EraseSubString(possibleAlias, ">");
            std::string lowTag = StringTools::tolower(tags[i]);
            if(myEffectAliases.contains(possibleAlias))
            {
                switch (myEffectAliases[possibleAlias]->myType)
                {
                case KaraokeEffect::Color:
                {
                    KaraokeColorEffect* effect = (KaraokeColorEffect*)myEffectAliases[possibleAlias];
                    if(effect->myHasEndColor) SetColor(effect->myStartColor, effect->myEndColor);
                    else SetColor(effect->myStartColor);
                    break;
                }
                }
                output = true;
            }
            else if(lowTag.starts_with("<font color"))
            {
                std::vector<std::string> colors = StringTools::Split(std::string(lowTag.data()), std::regex("[A-Za-z0-9 ]+"), true);
                if(colors.size() > 2) SetColor(FromHex(colors[1]), FromHex(colors[2]));
                if(colors.size() > 1) SetColor(FromHex(colors[1]));
                output = true;
            }
            else if(lowTag.starts_with("<no effect>"))
            {
                SetColor(myOverrideStartColor, myOverrideStartColor);
                output = true;
            }
        }
        return output;
    }
    void KaraokeDocument::SetColor(uint aStartColor)
    {
        myHasOverrideColor = true;
        myOverrideStartColor = aStartColor;
        myOverrideEndColor = myBaseEndColor;
    }
    void KaraokeDocument::SetColor(uint aStartColor, uint anEndColor)
    {
        myHasOverrideColor = true;
        myOverrideStartColor = aStartColor;
        myOverrideEndColor = anEndColor;
    }
    void KaraokeDocument::PopColor()
    {
        myHasOverrideColor = false;
        myOverrideStartColor = myBaseStartColor;
        myOverrideEndColor = myBaseEndColor;
    }
    void KaraokeDocument::InsertLineBreak(size_t aLineToSplit, size_t aToken, size_t aChar)
    {
        MakeDirty();
        myTokens.insert(myTokens.begin() + aLineToSplit, myTokens[aLineToSplit]);
        myTokens[aLineToSplit].erase(myTokens[aLineToSplit].begin() + aToken, myTokens[aLineToSplit].end());
        myTokens[aLineToSplit + 1].erase(myTokens[aLineToSplit + 1].begin(), myTokens[aLineToSplit + 1].begin() + aToken);
        if(aChar)
        {
            myTokens[aLineToSplit][aToken].myValue.erase(myTokens[aLineToSplit][aToken].myValue.begin() + aChar, myTokens[aLineToSplit][aToken].myValue.end());
            myTokens[aLineToSplit + 1][0].myValue.erase(myTokens[aLineToSplit][aToken].myValue.begin(), myTokens[aLineToSplit][aToken].myValue.begin() + aChar - 1);
        }
    }
    void KaraokeDocument::RevoveLineBreak(size_t aLineToMergeUp)
    {
        MakeDirty();
        if(aLineToMergeUp <= 0 || myTokens.size() <= aLineToMergeUp) return;
        myTokens[aLineToMergeUp - 1].insert(myTokens[aLineToMergeUp - 1].end(), myTokens[aLineToMergeUp].begin(), myTokens[aLineToMergeUp].end());
        myTokens.erase(myTokens.begin() + aLineToMergeUp);
    }
    void KaraokeDocument::MoveLineUp(size_t aLineToMove)
    {
        MakeDirty();
        if(aLineToMove <= 0 || myTokens.size() <= aLineToMove) return;
        myTokens[aLineToMove].swap(myTokens[aLineToMove - 1]);
    }
    void KaraokeDocument::DuplicateLine(size_t aLine)
    {
        MakeDirty();
        myTokens.insert(myTokens.begin() + aLine, myTokens[aLine]);
    }
    void KaraokeDocument::RemoveLine(size_t aLine)
    {
        MakeDirty();
        myTokens.erase(myTokens.begin() + aLine);
    }

    void KaraokeDocument::Clear()
    {
        for(int i = 0; i < myTokens.size(); i++)
        {
            myTokens[i].clear();
        }
        myTokens.clear();
    }
    void KaraokeDocument::Load(std::string aPath, std::string aFileID)
    {
        //if(aPath == myPath)
        //{
        //    printf("%s is already loaded!\n", aPath.c_str());
        //    return;
        //}
        printf("Loading %s.\n", aPath.c_str());
        if(!std::filesystem::exists(aPath))
        {
            printf("%s does not exist!\n", aPath.c_str());
            return;
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
        myFileID = aFileID;
        myName = std::filesystem::path(myPath).filename().string();
        std::ifstream docFile(aPath);
        std::string line;
        while(std::getline(docFile, line))
        {
            ParseLine(line);
        }
        docFile.close();
        if(!aPath.contains("local"))
        {
            std::filesystem::copy(aPath, "/local", std::filesystem::copy_options::overwrite_existing);
            FileHandler::SyncLocalFS();
        }
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
    void KaraokeDocument::ParseLineAndReplace(std::string aLine, size_t anIndex)
    {
        myTokens.erase(myTokens.begin() + anIndex);
        ParseLine(aLine);
        myTokens.insert(myTokens.begin() + anIndex, myTokens.back());
        myTokens.erase(myTokens.end());
    }
    void KaraokeDocument::ParseLine(std::string aLine)
    {
        if(aLine.starts_with("font"))
        {
            myFontSize = std::stoi(aLine.substr(5));
        }
        if(aLine.starts_with("start color"))
        {
            myHasBaseStartColor = true;
            myBaseStartColor = FromHex(aLine.substr(14));
            return;
        }
        if(aLine.starts_with("end color"))
        {
            myHasBaseEndColor = true;
            myBaseEndColor = FromHex(aLine.substr(12));
            return;
        }
        if(aLine.starts_with(".Resonate"))
        {
            // Use version to check if outdated.
            return;
        }
        if(aLine.starts_with(".Style"))
        {
            if(!aLine.contains("=")) return;
            std::vector<std::string> data = StringTools::Split(aLine, "=");
            std::string alias = data[0].substr(6);
            KaraokeEffect* effect = ParseEffectProperty(data[1]);
            myEffectAliases[alias] = effect;
            myECHOtoResonateAliases[effect->myECHOValue.data()] = "<" + alias + ">";
            return;
        }
        myTokens.push_back(std::vector<KaraokeToken>());
        ReplaceEffectsInLine(aLine);
        std::vector<std::string> rawTokens = StringTools::Split(aLine, std::regex("(\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\])?(.(?!(\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\])))*.?"), true);
        for(int i = 0; i < rawTokens.size(); i++)
        {
            std::vector<std::string> timeStamp = StringTools::Split(rawTokens[i], std::regex("\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\]"), true);
            std::vector<std::string> token = StringTools::Split(rawTokens[i], std::regex("\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\]"), false);
            if(timeStamp.size())
            {
                for(int t = 0; t < timeStamp.size(); t++)
                {
                    myTokens.back().push_back({!(token[t + 1].empty() || token[t + 1].contains('\0')) ? token[t + 1] : "", true, StringToTime(timeStamp[t])});
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
    void KaraokeDocument::ReplaceEffectsInLine(std::string& aLine)
    {
        std::vector<std::string> tags = StringTools::Split(aLine.data(), std::regex("<[A-Za-z0-9#\"= ]+>"), true);
        for(std::string tag : tags)
        {
            if(myECHOtoResonateAliases.contains(tag))
            {
                StringTools::Replace(aLine, tag, myECHOtoResonateAliases[tag]);
            }
        }
    }
    void KaraokeDocument::ReplaceAliasesInLine(std::string &aLine)
    {
        std::vector<std::string> tags = StringTools::Split(aLine.data(), std::regex("<[A-Za-z0-9#\"= ]+>"), true);
        for(std::string tag : tags)
        {
            std::string possibleAlias = tag;
            StringTools::EraseSubString(possibleAlias, "<");
            StringTools::EraseSubString(possibleAlias, ">");
            if(myEffectAliases.contains(possibleAlias))
            {
                StringTools::Replace(aLine, tag, myEffectAliases[possibleAlias]->myECHOValue);
            }
        }
    }
    std::string KaraokeDocument::Serialize()
    {
        std::string headers;
        std::string output;
        if(myFontSize != 50)
        {
            headers += (std::stringstream() << "font " << myFontSize << "\n").str();
        }
        if(myHasBaseStartColor)
        {
            headers += "start color 0x" + ToHex(myBaseStartColor) + "\n";
        }
        if(myHasBaseEndColor)
        {
            headers += "end color 0x" + ToHex(myBaseEndColor) + "\n";
        }
        headers += ".Resonate=1.1\n";
        for(auto&[alias, effect] : myEffectAliases)
        {
            headers += ".Style" + alias + "=" + SerializeEffectProperty(effect) + "\n";
        }
        for(int line = 0; line < myTokens.size(); line++)
        {
            for(int token = 0; token < myTokens[line].size(); token++)
            {
                output += (myTokens[line][token].myHasStart ? TimeToString(myTokens[line][token].myStartTime) : "") + myTokens[line][token].myValue;
            }
            output += "\n";
        }
        ReplaceAliasesInLine(output);
        return headers + output;
    }
    std::string KaraokeDocument::SerializeAsText()
    {
        std::string output;
        if(myHasBaseStartColor)
        {
            output += "start color 0x" + ToHex(myBaseStartColor) + "\n";
        }
        if(myHasBaseEndColor)
        {
            output += "end color 0x" + ToHex(myBaseEndColor) + "\n";
        }
        for(int line = 0; line < myTokens.size(); line++)
        {
            for(int token = 0; token < myTokens[line].size(); token++)
            {
                output += myTokens[line][token].myValue;
            }
            output += "\n";
        }
        return output;
    }
    std::string KaraokeDocument::SerializeLineAsText(KaraokeLine &aLine)
    {
        std::string output;
        for(int token = 0; token < aLine.size(); token++)
        {
            output += aLine[token].myValue;
        }
        output += "\n";
        return output;
    }
    std::string KaraokeDocument::Save()
    {
        std::ofstream docFile(myPath);
        docFile.clear();
        docFile << Serialize();
        docFile.close();
        return myPath;
    }
    std::string KaraokeDocument::AutoSave()
    {
        auto pathName = std::filesystem::path(myPath);
        std::ofstream docFile("/local/" + pathName.filename().string());
        docFile.clear();
        docFile << Serialize();
        printf("Auto saved to '/local/%s'.\n", pathName.filename().string().data());
        docFile.close();
        FileHandler::SyncLocalFS();
        myIsAutoDirty = false;
        return myPath;
    }
    bool KaraokeDocument::GetIsDirty()
    {
        return myIsDirty;
    }
    void KaraokeDocument::MakeDirty()
    {
        myLastEditTime = emscripten_get_now();
        myIsDirty = true;
        myIsAutoDirty = true;
    }
    void KaraokeDocument::UnsetIsDirty()
    {
        myIsDirty = false;
        myIsAutoDirty = false;
    }
    bool KaraokeDocument::GetIsAutoDirty()
    {
        return myIsAutoDirty && (emscripten_get_now() - myLastEditTime) > 2000;
    }
    // void KaraokeDocument::UnsetIsAutoDirty()
    //{
    //     myIsAutoDirty = false;
    // }
    std::string KaraokeDocument::GetPath()
    {
        return myPath;
    }
    std::string KaraokeDocument::GetFileID()
    {
        return myFileID;
    }
    std::string KaraokeDocument::GetName()
    {
        return myName;
    }
    KaraokeEffect *KaraokeDocument::ParseEffectProperty(std::string aRawProperty)
    {
        std::vector<std::string> data = StringTools::Split(aRawProperty, ",");
        printf("KaraokeDocument::ParseEffectProperty(%s)[data0=%s][data1=%s]\n", aRawProperty.data(), data[0].data(), data[1].data());
        switch (std::stoi(data[0]))
        {
        case KaraokeEffect::Color:
        {
            KaraokeColorEffect* effect = new KaraokeColorEffect();
            effect->myType = KaraokeEffect::Color;
            effect->myStartColor = FromHex(data[1]);
            effect->myHasEndColor = data.size() > 2;
            effect->myECHOValue = "<font color#" + data[1];
            if(effect->myHasEndColor)
            {
                effect->myEndColor = FromHex(data[2]);
                effect->myECHOValue += "#" + data[2];
            }
            effect->myECHOValue += ">";
            return effect;
        }
        }
        return nullptr;
    }
    std::string KaraokeDocument::SerializeEffectProperty(KaraokeEffect *aStyleProperty)
    {
        std::string output = "";
        switch (aStyleProperty->myType)
        {
        case KaraokeEffect::Color:
        {
            // (.StyleMiku=)1,00FFFFFF,30FFFFFF
            KaraokeColorEffect* effect = (KaraokeColorEffect*)aStyleProperty;
            output = "1," + ToHex(effect->myStartColor);
            if(effect->myHasEndColor)
            {
                output += "," + ToHex(effect->myEndColor);
            }
            break;
        }
        }
        return output;
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
        std::stringstream ss = std::stringstream();
        ss << "[" << std::setfill('0') << std::setw(2) << (aTime / 100) / 60 << ":" << std::setw(2) << (aTime / 100) % 60 << ":" << std::setw(2) << (aTime % 100) << "]";
        return ss.str();
    }
    uint KaraokeDocument::FromHex(std::string someHex)
    {
        uint output = 0;
        std::stringstream ss = std::stringstream();
        ss << std::hex << someHex;
        ss >> output;
        return output;
    }
    std::string KaraokeDocument::ToHex(uint aNum)
    {
        std::stringstream ss = std::stringstream();
        ss << std::hex << aNum;
        return ss.str();
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