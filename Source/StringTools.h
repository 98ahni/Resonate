#pragma once
#include <string>
#include <vector>
#include <regex>

namespace StringTools
{
	std::vector<std::string> Split(std::string aStringToSplit, std::string aSplitter);
	std::vector<std::string> Split(std::string aStringToSplit, std::regex aSplitPattern, bool aGetMatching = false);
	void EraseSubString(std::string& aStringToTrim, std::string aStringToErase);
	void Replace(std::string& aBaseString, std::string aStringToReplace, std::string aReplacement);
	std::string tolower(const std::string& aString);
	std::string TOUPPER(const std::string& aString);
}
