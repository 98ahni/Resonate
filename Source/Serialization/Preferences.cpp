//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Preferences.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <Defines.h>

EM_JS(void, load_preferences_json, (), {
    if(!FS.analyzePath('/local/Prefs.Resonate').exists)
    {
        FS.writeFile('/local/Prefs.Resonate', '{}');
    }
    global_preferences = JSON.parse(FS.readFile('/local/Prefs.Resonate', { encoding: 'utf8' }));
}
var global_preferences = {};
);
EM_JS(void, print_preferences_json, (), {
    console.log(JSON.stringify(global_preferences));
});

EM_JS(emscripten::EM_VAL, set_preference_value, (emscripten::EM_VAL key, emscripten::EM_VAL value), {
    global_preferences[Emval.toValue(key)] = Emval.toValue(value);
    FS.writeFile('/local/Prefs.Resonate', JSON.stringify(global_preferences));
    return Emval.toHandle(new Promise((resolve)=>{FS.syncfs(false, function (err) {
        if(err){
            console.error('Unable to sync IndexDB!\n' + err);
        }
        resolve();
    })}));
});

EM_JS(emscripten::EM_VAL, get_preference_value, (emscripten::EM_VAL key), {
    return Emval.toHandle(global_preferences[Emval.toValue(key)]);
});

EM_JS(emscripten::EM_VAL, has_preference_key, (emscripten::EM_VAL key), {
    return Emval.toHandle(global_preferences.hasOwnProperty(Emval.toValue(key)));
});

void Serialization::LoadPrefs()
{
    load_preferences_json();
}

void Serialization::PrintPrefs()
{
    print_preferences_json();
}

void Serialization::Preferences::SetBool(std::string aKey, bool someValue)
{
    VAR_FROM_JS(set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue))).await();
}
void Serialization::Preferences::SetInt(std::string aKey, int someValue)
{
    VAR_FROM_JS(set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue))).await();
}
void Serialization::Preferences::SetUint(std::string aKey, uint32_t someValue)
{
    VAR_FROM_JS(set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue))).await();
}
void Serialization::Preferences::SetFloat(std::string aKey, float someValue)
{
    VAR_FROM_JS(set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue))).await();
}
void Serialization::Preferences::SetDouble(std::string aKey, double someValue)
{
    VAR_FROM_JS(set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue))).await();
}
void Serialization::Preferences::SetString(std::string aKey, std::string someValue)
{
    VAR_FROM_JS(set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue))).await();
}

bool Serialization::Preferences::HasKey(std::string aKey)
{
    return VAR_FROM_JS(has_preference_key(VAR_TO_JS(aKey))).as<bool>();
}

bool Serialization::Preferences::GetBool(std::string aKey)
{
    return VAR_FROM_JS(get_preference_value(VAR_TO_JS(aKey))).as<bool>();
}
int Serialization::Preferences::GetInt(std::string aKey)
{
    return VAR_FROM_JS(get_preference_value(VAR_TO_JS(aKey))).as<int>();
}
uint32_t Serialization::Preferences::GetUint(std::string aKey)
{
    return VAR_FROM_JS(get_preference_value(VAR_TO_JS(aKey))).as<uint32_t>();
}
float Serialization::Preferences::GetFloat(std::string aKey)
{
    return VAR_FROM_JS(get_preference_value(VAR_TO_JS(aKey))).as<float>();
}
double Serialization::Preferences::GetDouble(std::string aKey)
{
    return VAR_FROM_JS(get_preference_value(VAR_TO_JS(aKey))).as<double>();
}
std::string Serialization::Preferences::GetString(std::string aKey)
{
    return VAR_FROM_JS(get_preference_value(VAR_TO_JS(aKey))).as<std::string>();
}
