#pragma once
#include "storage.h"
const char* defaultUpdateUrl();
std::string resolveUpdateSource(const std::string&config);
bool parseGitHubFile(const std::string&json,Release&release,std::string&error);
bool verifyReleaseFile(const std::string&path,const Release&release);
