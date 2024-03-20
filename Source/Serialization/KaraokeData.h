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
        void SetColor(uint aStartColor);
        void SetColor(uint aStartColor, uint anEndColor);
        void PopColor();

        void Clear();
        void Load(std::string aPath);
        void Parse(std::string aDocument);
        void ParseLineAndReplace(std::string aLine, size_t anIndex);
        std::string Serialize();
        std::string SerializeAsText();
        std::string SerializeLineAsText(KaraokeLine& aLine);
        std::string Save();

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
        KaraokeData myTokens;
        std::string myPath;
        bool myHasBaseStartColor = false;
        uint myBaseStartColor = 0xFF00E600;
        bool myHasBaseEndColor = false;
        uint myBaseEndColor = 0x9FFF80BF;
        bool myHasOverrideColor = false;
        uint myOverrideStartColor = 0xFF00E600;
        uint myOverrideEndColor = 0x9FFF80BF;
    };
}