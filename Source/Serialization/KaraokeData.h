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

        void Clear();
        void Load(std::string aPath);
        void Parse(std::string aDocument);
        std::string Serialize();
        std::string Save();

        static uint StringToTime(std::string aTimeStr);
        static std::string TimeToString(uint aTime);

    private:
        void ParseLine(std::string aLine);

        static KaraokeDocument* ourInstance;
        inline static KaraokeToken ourNullToken = {"", false, 0xFFFFFFFF};
        KaraokeData myTokens;
        std::string myPath;
    };
}