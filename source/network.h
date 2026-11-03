#pragma once
#include "storage.h"
using Progress=std::function<bool(uint64_t,uint64_t)>;
struct FetchOptions {
    uint64_t timeoutNs=15000000000ull;
    uint64_t budgetNs=0;
    uint64_t pollNs=0;
    std::function<bool()> cancelled;
    std::function<uint64_t()> nowNs;
};
bool fetch(const std::string&url,const std::string&path,uint64_t limit,uint64_t exact,std::string&error,const Progress&progress,const FetchOptions& options={});
