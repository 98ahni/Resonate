#include "StringTools.h"

namespace StringTools
{
	std::vector<std::string> Split(std::string aStringToSplit, std::string aSplitter)
	{
		std::vector<std::string> output{};

		size_t index = 0;
		while ((index = aStringToSplit.find(aSplitter)) != std::string::npos)
		{
			output.push_back(aStringToSplit.substr(0, index));
			aStringToSplit.erase(0, index + aSplitter.length());
		}
		output.push_back(aStringToSplit);
		return output;
	}

    std::vector<std::string> Split(std::string aStringToSplit, std::regex aSplitPattern, bool aGetMatching)
    {
        return {std::sregex_token_iterator(aStringToSplit.begin(), aStringToSplit.end(), aSplitPattern, aGetMatching ? 0 : -1), std::sregex_token_iterator()};
    }

    void EraseSubString(std::string& aStringToTrim, std::string aStringToErase)
	{
		size_t index = 0;
		while ((index = aStringToTrim.find(aStringToErase)) != std::string::npos)
		{
			aStringToTrim.erase(index, aStringToErase.length());
		}
	}

	void Replace(std::string& aBaseString, std::string aStringToReplace, std::string aReplacement)
	{
		std::vector<std::string> cutStr = Split(aBaseString, aStringToReplace);
		aBaseString.clear();
		aBaseString = cutStr[0];
		for (int i = 1; i < cutStr.size(); i++)
		{
			aBaseString += aReplacement + cutStr[i];
		}
	}

	std::string tolower(const std::string& aString)
	{
		std::string output;
		for (int i = 0; i < aString.size(); i++)
		{
			output += (char)std::tolower(aString[i]);
		}
		return output;
	}

	std::string TOUPPER(const std::string& aString)
	{
		std::string output;
		for (int i = 0; i < aString.size(); i++)
		{
			output += (char)std::toupper(aString[i]);
		}
		return output;
	}
}