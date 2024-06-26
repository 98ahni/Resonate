//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include <string>

namespace Serialization
{
    void LoadPrefs();
    void PrintPrefs();

    namespace Preferences
    {
        void SetBool(std::string aKey, bool someValue);
        void SetInt(std::string aKey, int someValue);
        void SetUint(std::string aKey, uint32_t someValue);
        void SetFloat(std::string aKey, float someValue);
        void SetDouble(std::string aKey, double someValue);
        void SetString(std::string aKey, std::string someValue);
        
        bool HasKey(std::string aKey);

        bool GetBool(std::string aKey);
        int GetInt(std::string aKey);
        uint32_t GetUint(std::string aKey);
        float GetFloat(std::string aKey);
        double GetDouble(std::string aKey);
        std::string GetString(std::string aKey);
    }
}