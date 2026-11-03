#pragma once
#include "storage.h"
using Progress=std::function<bool(uint64_t,uint64_t)>;
bool fetch(const std::string&url,const std::string&path,uint64_t limit,uint64_t exact,std::string&error,const Progress&progress);
