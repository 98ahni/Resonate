#include "Syllabify.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <Defines.h>
#include <filesystem>

EM_JS(emscripten::EM_VAL, language_code_to_name, (emscripten::EM_VAL language_code),{
    let names = new Intl.DisplayNames(['en-GB'/*, Emval.toValue(language_code)*/], {type:"language"});
    return Emval.toHandle(names.of(Emval.toValue(language_code).replace('_', '-')));//.toString());
});

namespace Serialization
{
    struct PatternData
    {
        inline static std::map<std::string, std::string> myLanguages;
        static inline std::vector<std::unordered_map<std::string, std::string>> myPatterns;
        inline static std::vector<int> myMaxPatternSize;
    };

    void Syllabify_Init()
    {
	    for(const auto& p : std::filesystem::directory_iterator("Syllabify/"))
        {
            PatternData::myLanguages[p.path().stem().string()] = VAR_FROM_JS(language_code_to_name(VAR_TO_JS(p.path().stem().string()))).as<std::string>();
        }
    }

    void BuildPatterns(std::string aLanguageCode)
    {
        PatternData::myPatterns.clear();
        PatternData::myMaxPatternSize.clear();
        PatternData::myPatterns.push_back(std::unordered_map<std::string, std::string>());
        PatternData::myMaxPatternSize.push_back(0);

        DBGprintf("Loading language file %s\n", SYLLABIFY_PATHSTD(aLanguageCode).data());
        std::ifstream file(SYLLABIFY_PATHSTD(aLanguageCode).data());
        if (!file)
            return;

        DBGprintf("Building pattern %s.\n", aLanguageCode.data());
        std::string line;
        while (getline(file, line))
            BuildPattern(line);

        DBGprintf("Pattern %s built.\n", aLanguageCode.data());
        file.close();
    }

    void BuildPattern(std::string aLine, int i)
    {
        while (i < aLine.size() && isspace(aLine[i]))
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
            else if (isspace(aLine[j]) || aLine[j] == '#' || aLine[j] == '%')
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
    
    std::string ApplyPatterns(std::string aText, int aLevel)
    {
        DBGprintf("Applying pattern to \"%s\" with level %i.\n", aText.data(), aLevel);
        const std::string wrappedWord = '.' + aText + '.';
        std::string splits;
        splits.resize(aText.size() - 1);

        for (int i = 0; i < wrappedWord.size(); ++i)
        {
            for (int j = 1; j <= wrappedWord.size() - i && j <= PatternData::myMaxPatternSize[aLevel]; ++j)
            {
                if (i + j < wrappedWord.size() && iscntrl(static_cast<unsigned char>(wrappedWord[i + j - 1])))
                    continue;  // A pattern that ends with "a" must not match "ä"

                const auto it = PatternData::myPatterns[aLevel].find(wrappedWord.substr(i, j));
                if (it != PatternData::myPatterns[aLevel].end())
                {
                    const std::string& pattern = it->second;
                    for (int k = 0; k < pattern.size(); ++k)
                    {
                        const int l = i - 2 + k;
                        if (l >= 0 && l < splits.size())
                        splits[l] = std::max(splits[l], pattern[k]);
                    }
                }
            }
        }

        if (aLevel + 1 < PatternData::myPatterns.size())
        {
            int subwordStart = 0;

            for (int i = 0; i < splits.size(); ++i)
            {
                if (splits[i] % 2 == 1)
                {
                    splits.replace(subwordStart, i - subwordStart,
                                   ApplyPatterns(aText.substr(subwordStart, i + 1 - subwordStart), aLevel));
                    subwordStart = i + 1;
                }
            }

            if (subwordStart == 0)
            {
                return ApplyPatterns(aText, aLevel + 1);
            }
            else
            {
                splits.replace(subwordStart, splits.size() - subwordStart,
                               ApplyPatterns(aText.substr(subwordStart, aText.size() - subwordStart), aLevel));
            }
        }

        DBGprintf("ApplyPattern returned \"%s\".\n", splits.data());
        return splits;
    }

    std::string ApplyPatterns(std::vector<std::string> someText, int aLevel)
    {
        std::string input;
        for(const std::string& txt : someText)
        {
           input += txt;
        }
        ApplyPatterns(input, aLevel);
    }
    
    void SyllabifyWord(std::vector<int>& someSplitPoints, std::string aPartialText, int aStartIndex)
    {
        DBGprintf("Syllabifying word \"%s\".\n", aPartialText.data());
        std::vector<int> index_mapping;
        for (int j = 0; j < aPartialText.size(); j++)
        {
            index_mapping.push_back(aStartIndex + j);
        }

        std::string normalized_word;
        for(int j = 0; j < aPartialText.size(); j++)
        {
            normalized_word += tolower(aPartialText[j]);
        }
        if (index_mapping.size() != normalized_word.size())
        {
            //error
            printf("Index map doesn't match word!\n");
        }

        const std::string splits = ApplyPatterns(normalized_word);
        for (int i = 0; i < splits.size(); ++i)
        {
            if (splits[i] % 2 == 1)
                someSplitPoints.push_back(index_mapping[i + 1]);
        }
    }
    std::vector<std::string> Syllabify(std::string aText, std::string aLanguageCode)
    {
        std::vector<int> splitPoints;

        if (aText.empty())
            return {};

        splitPoints.push_back(0);

        //QChar::Script prevScript = QChar::Script_Unknown;
        bool lastIsAlnum = false;
        bool lastIsNum = false;
        int wordBeforeStart = 0;
        int wordStart = 0;
        for (int i = 0; i < aText.size(); ++i)
        {
            //if (IsHighSurrogate(text, i))
            //    continue;

            char currentChar = aText[i];

            //if (ispunct(static_cast<unsigned char>(currentChar)))
            //    continue;

            //const QChar::Script script = QChar::script(currentChar);
            bool isAlNum = isalnum(currentChar);
            bool isNum = isdigit(currentChar);
            //const bool scriptsMatch = isNum == lastIsNum &&
            //        (previousScript == script || script <= 2 || previousScript <= 2);

            //const int currentCodepoint = IsLowSurrogate(text, i) ? i - 1 : i;

            if (!isAlNum /*|| !scriptsMatch)*/ && lastIsAlnum)
            {
                if (wordBeforeStart < 0)
                    splitPoints.push_back(wordStart);
                else if (wordBeforeStart != 0)
                    splitPoints.push_back(wordBeforeStart);

                if (!lastIsNum)
                    SyllabifyWord(splitPoints, aText.substr(wordStart, (i/* - 1*/) - wordStart), wordStart);

                wordBeforeStart = -1;
            }

            if (!lastIsAlnum /*|| !scriptsMatch*/)
                wordStart = i;// - 1;

            if (isspace(currentChar))
                wordBeforeStart = i + 1;

            //previousScript = script;
            lastIsAlnum = isAlNum;
            lastIsNum = isNum;
        }

        if (lastIsAlnum)
        {
            if (wordBeforeStart < 0)
                splitPoints.push_back(wordStart);
            else if (wordBeforeStart != 0)
                splitPoints.push_back(wordBeforeStart);

            SyllabifyWord(splitPoints, aText.substr(wordStart, aText.size() - wordStart), wordStart);
        }

        splitPoints.push_back(aText.size());

        std::vector<std::string> output;
        DBGprintf("Amount of splits: %i.\n", splitPoints.size());
        for (int i = 1; i < splitPoints.size(); ++i)
        {
            output.push_back(aText.substr(splitPoints[i - 1], splitPoints[i] - splitPoints[i - 1]));
        }
        return output;
    }

    std::vector<std::string> Syllabify(std::vector<std::string> someText, std::string aLanguageCode)
    {
        std::vector<std::string> output;
        for(const std::string& txt : someText)
        {
            std::vector<std::string> result = Syllabify(txt, aLanguageCode);
            output.insert(output.end(), result.begin(), result.end());
        }
    }
    std::map<std::string, std::string> GetAvailableLanguages()
    {
        return PatternData::myLanguages;
    }
}