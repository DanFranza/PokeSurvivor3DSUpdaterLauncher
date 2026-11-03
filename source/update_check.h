#pragma once
#include <atomic>
#include <functional>
#include <string>
#include <cstdint>
#ifdef __3DS__
#include <3ds.h>
#else
#include <thread>
#endif

bool hasGameUpdate(const std::string& installed,const std::string& available);
bool readUpdateCache(const std::string& text,const std::string& source,int64_t now,std::string& version);
std::string makeUpdateCache(const std::string& source,int64_t now,const std::string& version);

class UpdateChecker {
public:
    using Query=std::function<bool(const std::string&,const std::function<bool()>&,std::string&)>;
    explicit UpdateChecker(Query query,std::function<int64_t()> clock={});
    ~UpdateChecker();
    UpdateChecker(const UpdateChecker&)=delete;
    UpdateChecker& operator=(const UpdateChecker&)=delete;
    bool start(const std::string& source,const std::string& cache);
    void cancel();
    bool running()const;
    void collect();
    void clearResult();
    std::string latestVersion()const;
private:
    void run();
    static void entry(void* pointer);
    Query query_;
    std::function<int64_t()> clock_;
    std::string source_,cache_,latest_;
    std::atomic<bool> cancel_{false},done_{true};
#ifdef __3DS__
    Thread worker_=nullptr;
#else
    std::thread worker_;
#endif
};
