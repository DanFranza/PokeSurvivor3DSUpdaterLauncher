#pragma once
#include <initializer_list>
#include <string>

enum class Language { French, English, Spanish, Italian, Portuguese };
constexpr int LANGUAGE_COUNT = 5;
Language currentLanguage();
void setLanguage(Language language);
const char* languageCode(Language language);
const char* languageName(Language language);
bool parseLanguageCode(const std::string& code, Language& language);
const char* tr(const char* message);
std::string tr(const std::string& message);
std::string trf(const char* message, std::initializer_list<std::string> values);
bool loadLanguagePreference(const std::string& path);
bool saveLanguagePreference(const std::string& path);
