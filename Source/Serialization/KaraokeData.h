#include <string>
#include <vector>

typedef unsigned int uint;
namespace Serialization 
{
    struct KaraokeToken
    {
        std::string myValue;
        bool myHasStart;
        uint myStartTime;
    };
    typedef std::vector<std::vector<KaraokeToken>> KaraokeData;
    typedef std::vector<KaraokeToken> KaraokeLine;
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
        bool IsPauseToken(size_t aLine, size_t aToken);
        bool IsPauseToken(KaraokeToken& aToken);
        uint GetStartColor();
        uint GetEndColor();
        bool ParseEffectToken(KaraokeToken& aToken);
        void SetColor(uint aStartColor);
        void SetColor(uint aStartColor, uint anEndColor);
        void PopColor();
        void InsertLineBreak(size_t aLineToSplit, size_t aToken, size_t aChar);
        void RevoveLineBreak(size_t aLineToMergeUp);
        void MoveLineUp(size_t aLineToMove);
        void DuplicateLine(size_t aLine);
        void RemoveLine(size_t aLine);

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

        static uint StringToTime(std::string aTimeStr);
        static std::string TimeToString(uint aTime);
        static uint FromHex(std::string someHex);
        static std::string ToHex(uint aNum);
        static bool IsNull(KaraokeToken& aToken);
        static bool IsNull(KaraokeLine& aLine);

    private:
        void ParseLine(std::string aLine);

        static KaraokeDocument* ourInstance;
        inline static KaraokeToken ourNullToken = {"", false, 0};
        static inline KaraokeLine ourNullLine = KaraokeLine();
        
        std::string myPath;
        std::string myFileID;
        std::string myName;
        bool myIsDirty = false;
        bool myIsAutoDirty = false; // Set immediatly
        double myLastEditTime = 0;

        KaraokeData myTokens;
        bool myHasBaseStartColor = false;
        uint myBaseStartColor = 0xFF00E600;
        bool myHasBaseEndColor = false;
        uint myBaseEndColor = 0x9FFF80BF;
        bool myHasOverrideColor = false;
        uint myOverrideStartColor = 0xFF00E600;
        uint myOverrideEndColor = 0x9FFF80BF;
    };
}