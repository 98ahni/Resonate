#include <string>
#include <vector>

namespace Serialization 
{
    struct KaraokeToken
    {
        std::string myValue;
        float myStartTime;
        float myEndTime;
    };
    typedef std::vector<std::vector<KaraokeToken>> KaraokeData;
    typedef std::vector<KaraokeToken> KaraokeLine;
    class KaraokeDocument
    {
    public:
        static KaraokeDocument& Get();
        KaraokeData& GetData();
        KaraokeLine& GetLine(size_t i);

    private:
        static KaraokeDocument* ourInstance;
        KaraokeData myTokens;
    };
}