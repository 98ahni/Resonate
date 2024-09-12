//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include <string>
#include <vector>
#include <unordered_map>

typedef unsigned int uint;
class PropertiesWindow;
namespace Serialization 
{
    struct KaraokeEffect
    {
        enum EffectType
        {
            None,
            Color,
            Image,
            Raw
        };
        std::string myECHOValue;
        EffectType myType;
    };
    struct KaraokeColorEffect : public KaraokeEffect
    {
        uint myStartColor = 0x0038F97C;
        bool myHasEndColor = false;
        uint myEndColor = 0x30FFCCE9;
    };
    struct KaraokeImageEffect : public KaraokeEffect
    {
        std::string myImageName;
    };
    struct KaraokeToken
    {
        std::string myValue;
        bool myHasStart;
        uint myStartTime;
    };
    typedef std::vector<std::vector<KaraokeToken>> KaraokeData;
    typedef std::vector<KaraokeToken> KaraokeLine;
    typedef std::unordered_map<std::string, KaraokeEffect*> KaraokeAliasMap;
    class KaraokeDocument
    {
    public:
        static KaraokeDocument& Get();
        KaraokeData& GetData();
        KaraokeLine& GetLine(size_t aLine);
        KaraokeLine& GetValidLineAfter(size_t aLine);
        KaraokeLine& GetValidLineBefore(size_t aLine);
        KaraokeToken& GetToken(size_t aLine, size_t aToken);
        KaraokeToken& GetTokenAfter(size_t aLine, size_t aToken);
        KaraokeToken& GetTokenBefore(size_t aLine, size_t aToken);
        KaraokeToken& GetTimedTokenAfter(size_t aLine, size_t aToken);
        KaraokeToken& GetTimedTokenBefore(size_t aLine, size_t aToken);
        KaraokeToken& GetThisOrNextTimedToken(size_t aLine, size_t aToken);
        KaraokeToken& GetThisOrPreviousTimedToken(size_t aLine, size_t aToken);
        bool IsPauseToken(size_t aLine, size_t aToken);
        bool IsPauseToken(KaraokeToken& aToken);
        uint GetFontSize();
        uint GetStartColor();
        uint GetEndColor();
        bool IsEffectToken(KaraokeToken& aToken);
        bool ParseEffectToken(KaraokeToken& aToken);
        void SetColor(uint aStartColor);
        void SetColor(uint aStartColor, uint anEndColor);
        void PopColor();
        void InsertLineBreak(size_t aLineToSplit, size_t aToken, size_t aChar);
        void RevoveLineBreak(size_t aLineToMergeUp);
        void MoveLineUp(size_t aLineToMove);
        void DuplicateLine(size_t aLine);
        void RemoveLine(size_t aLine);
        void ShiftTimings(int aTimeShift);
        const KaraokeAliasMap& GetEffectAliases();

        void Clear();
        void Load(std::string aPath, std::string aFileID = "");
        void Parse(std::string aDocument);
        void ParseLineAndReplace(std::string aLine, size_t anIndex);
        std::string Serialize();
        std::string SerializeAsText();
        std::string SerializeLineAsText(KaraokeLine& aLine);
        std::string Save();
        std::string AutoSave();
        bool GetIsDirty();          // Has the user saved since last edit
        void MakeDirty();           // Has an edit occured 
        void UnsetIsDirty();        // Has a save occured 
        bool GetIsAutoDirty();      // Has the document been auto saved since last edit and passed a cooldown
        //void UnsetIsAutoDirty();  // Has an auto save occured (Set in AutoSave())
        std::string GetPath();
        std::string GetFileID();
        std::string GetName();

        static KaraokeEffect* ParseEffectProperty(std::string aRawProperty);
        static std::string SerializeEffectProperty(KaraokeEffect* aStyleProperty);
        static uint StringToTime(std::string aTimeStr);
        static std::string TimeToString(uint aTime);
        static uint FromHex(std::string someHex);
        static std::string ToHex(uint aNum);
        static bool IsNull(KaraokeToken& aToken);
        static bool IsNull(KaraokeLine& aLine);

    private:
        void ParseLine(std::string aLine);
        void ReplaceEffectsInLine(std::string& aLine);
        void ReplaceAliasesInLine(std::string& aLine);

        static KaraokeDocument* ourInstance;
        static KaraokeToken ourNullToken;
        static KaraokeLine ourNullLine;
        
        std::string myPath;
        std::string myFileID;
        std::string myName;
        bool myIsDirty = false;
        bool myIsAutoDirty = false; // Set immediatly
        double myLastEditTime = 0;

        KaraokeData myTokens;
        uint myFontSize = 50;
        bool myHasBaseStartColor = false;
        uint myBaseStartColor = 0x0038F97C;
        bool myHasBaseEndColor = false;
        uint myBaseEndColor = 0x30FFCCE9;
        bool myHasOverrideColor = false;
        uint myOverrideStartColor = 0x0038F97C;
        uint myOverrideEndColor = 0x30FFCCE9;
        std::unordered_map<std::string, std::string> myECHOtoResonateAliases;
        KaraokeAliasMap myEffectAliases;
        friend class ::PropertiesWindow;
    };
}