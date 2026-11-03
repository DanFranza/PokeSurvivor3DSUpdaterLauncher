#pragma once
#include <cstdint>
#include <string>
enum class GameFormat { ThreeDSX, CIA };
constexpr uint64_t GAME_CIA_TITLE_ID=0x000400000ed53100ull;
const char* gameFormatCode(GameFormat format);
bool parseGameFormat(const std::string& text,GameFormat& format);
bool validGameCia(const std::string& path);
