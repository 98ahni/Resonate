//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#pragma once
#include <string>
#include <vector>
#include <regex>

namespace StringTools
{
	std::vector<std::string> Split(std::string aStringToSplit, std::string aSplitter);
	std::vector<std::string> Split(std::string aStringToSplit, std::regex aSplitPattern, bool aGetMatching = false);
	std::string Join(std::vector<std::string> aStringList, std::string aJoiner = "");
	void EraseSubString(std::string& aStringToTrim, std::string aStringToErase);
	void Replace(std::string& aBaseString, std::string aStringToReplace, std::string aReplacement);
	std::string tolower(const std::string& aString);
	std::string TOUPPER(const std::string& aString);
}
