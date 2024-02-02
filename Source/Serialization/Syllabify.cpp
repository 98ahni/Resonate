#include "Syllabify.h"
#include <vector>
#include <unordered_map>
#include <fstream>
#include <Defines.h>

namespace Serialization
{
    struct PatternData
    {
        static inline std::vector<std::unordered_map<std::string, std::string>> myPatterns;
        inline static std::vector<int> myMaxPatternSize;
    };

    void BuildPatterns(std::string aLanguageCode)
    {
        PatternData::myPatterns.push_back(std::unordered_map<std::string, std::string>());
        PatternData::myMaxPatternSize.push_back(0);

        std::ifstream file(SYLLABIFY_PATHSTD(aLanguageCode).data());
        if (!file)
            return;

        std::string line;
        while (getline(file, line))
            BuildPattern(line);

        file.close();
    }

    void BuildPattern(std::string aLine, int i)
    {
        while (i < aLine.size() && aLine[i] == ' ')
            ++i;

        if (i >= aLine.size() || aLine[i] == '#' || aLine[i] == '%')
            return;

        if (isupper(aLine[i]))
        {
            if (aLine == "NEXTLEVEL")
            {
                PatternData::myPatterns.push_back(std::unordered_map<std::string, std::string>());
                PatternData::myMaxPatternSize.push_back(0);
            }
            return;
        }

        size_t letters = 0;
        int lineSize = aLine.size();
        for (int j = i; j < lineSize; ++j)
        {
            if (aLine[j] == '/')
            {
                // A slash in the middle of a line indicates the start of a Hunspell Hyphen non-standard
                // hyphenation pattern (e.g. tillåta -> till-låta). Since we are not doing hyphenation,
                // we just treat the line as if the slash and everything after it doesn't exist.
                lineSize = j;
            }
            else if (aLine[j] == ' ' || aLine[j] == '#' || aLine[j] == '%')
            {
                // In TeX files but seemingly not Hunspell Hyphen files, spaces can be used to separate
                // patterns on the same line, and a comment can start in the middle of a line.
                lineSize = j;
                BuildPattern(aLine, j);
            }
            else if (aLine[j] < '0' || aLine[j] > '9')
            {
                letters++;
            }
        }

        std::string key;
        key.resize(letters);
        std::string value;
        value.resize(letters + 1);

        for (int j = i, k = 0; j < lineSize; ++j)
        {
            if (aLine[j] < '0' || aLine[j] > '9')
                key[k++] = aLine[j];
            else
                value[k] = aLine[j];
        }

        const auto it = PatternData::myPatterns.back().find(key);
        if (it != PatternData::myPatterns.back().end())
        {
            std::string other_value = it->second;
            for (int i = 0; i < value.size(); ++i)
                value[i] = std::max(value[i], other_value[i]);
        }

        PatternData::myPatterns.back()[key] = value;
        PatternData::myMaxPatternSize.back() = std::max<int>(PatternData::myMaxPatternSize.back(), key.size());
    }
}