#include "i18n.h"
#include "storage.h"
#include <cstring>

namespace {
Language selectedLanguage = Language::French;
const char* codes[LANGUAGE_COUNT] = {"fr", "en", "es", "it", "pt"};
const char* names[LANGUAGE_COUNT] = {"Français", "English", "Español", "Italiano", "Português"};
const char* messages[][LANGUAGE_COUNT] = {
#include "translations.inc"
};
int index(Language language) {
    int value = static_cast<int>(language);
    return value >= 0 && value < LANGUAGE_COUNT ? value : 0;
}
}

Language currentLanguage() { return selectedLanguage; }
void setLanguage(Language language) { selectedLanguage = static_cast<Language>(index(language)); }
const char* languageCode(Language language) { return codes[index(language)]; }
const char* languageName(Language language) { return names[index(language)]; }
bool parseLanguageCode(const std::string& text, Language& language) {
    auto code = text;
    while (!code.empty() && (code.back() == '\n' || code.back() == '\r')) code.pop_back();
    for (int i = 0; i < LANGUAGE_COUNT; ++i) {
        if (code == codes[i]) { language = static_cast<Language>(i); return true; }
    }
    return false;
}
const char* tr(const char* message) {
    for (const auto& row : messages) {
        if (strcmp(row[0], message) == 0) return row[index(selectedLanguage)];
    }
    return message;
}
std::string tr(const std::string& message) { return std::string(tr(message.c_str())); }
std::string trf(const char* message, std::initializer_list<std::string> values) {
    std::string text = tr(message);
    int number = 0;
    for (const auto& value : values) {
        std::string marker = "{" + std::to_string(number++) + "}";
        size_t at = 0;
        while ((at = text.find(marker, at)) != std::string::npos) {
            text.replace(at, marker.size(), value);
            at += value.size();
        }
    }
    return text;
}
bool loadLanguagePreference(const std::string& path) {
    Language language;
    if (!parseLanguageCode(readText(path), language)) return false;
    setLanguage(language);
    return true;
}
bool saveLanguagePreference(const std::string& path) {
    return writeText(path, languageCode(currentLanguage()));
}
