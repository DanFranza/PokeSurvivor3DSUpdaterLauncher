#pragma once
#include "storage.h"
const char* defaultUpdateUrl();
const char* ciaUpdateUrl();
std::string resolveUpdateSource(const std::string&config);
bool parseUpdateMetadata(const std::string& metadata,Release& release,std::string& error,GameFormat format=GameFormat::ThreeDSX);
bool parseGitHubFile(const std::string&json,Release&release,std::string&error,GameFormat format=GameFormat::ThreeDSX);
bool verifyReleaseFile(const std::string&path,const Release&release);
