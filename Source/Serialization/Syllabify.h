#include <string>
#include <vector>

namespace Serialization
{
    void BuildPatterns(std::string aLanguageCode);
    void BuildPattern(std::string aLine, int i = 0);
    std::string ApplyPatterns(std::string aText, int aLevel = 0);
    std::string ApplyPatterns(std::vector<std::string> someText, int aLevel = 0);
    std::vector<std::string> Syllabify(std::string aText, std::string aLanguageCode);
    std::vector<std::string> Syllabify(std::vector<std::string> someText, std::string aLanguageCode);
}