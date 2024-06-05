#include "Preferences.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <Defines.h>

EM_JS(void, load_preferences_json, (), {
    global_preferences = JSON.parse(FS.readFile('/local/.Resonate', { encoding: 'utf8' }));
}
var global_preferences = {};
);

EM_JS(void, set_preference_value, (emscripten::EM_VAL key, emscripten::EM_VAL value), {
    global_preferences[Emval.toValue(key)] = Emval.toValue(value);
    FS.writeFile('/local/.Resonate', JSON.stringify(global_preferences));
    FS.syncFS();
});

EM_JS(emscripten::EM_VAL, get_preference_value, (emscripten::EM_VAL key), {
    return Emval.toHandle(global_preferences[Emval.toValue(key)]);
});

void Serialization::LoadPrefs()
{
    load_preferences_json();
}

void Serialization::Preferences::SetBool(std::string aKey, bool someValue)
{
    set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue));
}
void Serialization::Preferences::SetInt(std::string aKey, int someValue)
{
    set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue));
}
void Serialization::Preferences::SetUint(std::string aKey, uint32_t someValue)
{
    set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue));
}
void Serialization::Preferences::SetFloat(std::string aKey, float someValue)
{
    set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue));
}
void Serialization::Preferences::SetDouble(std::string aKey, double someValue)
{
    set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue));
}
void Serialization::Preferences::SetString(std::string aKey, std::string someValue)
{
    set_preference_value(VAR_TO_JS(aKey), VAR_TO_JS(someValue));
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
