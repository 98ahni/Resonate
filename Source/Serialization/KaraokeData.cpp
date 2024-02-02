#include "KaraokeData.h"

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
}