#include "update_check.h"
#include "storage.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <future>
#include <iostream>
#include <cstdlib>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"Background check failed at "<<__LINE__<<": "<<#x<<"\n";std::abort();}}while(0)
static void finish(UpdateChecker& check){auto end=std::chrono::steady_clock::now()+std::chrono::seconds(5);while(check.running()&&std::chrono::steady_clock::now()<end)std::this_thread::yield();CHECK(!check.running());check.collect();}
int main(){
    const std::string source="https://api.github.com/game",other="https://api.github.com/other";std::string version;
    CHECK(hasGameUpdate("3ds-old","3ds-new"));CHECK(!hasGameUpdate("3ds-new","3ds-new"));CHECK(!hasGameUpdate("inconnue","3ds-new"));CHECK(!hasGameUpdate("","3ds-new"));CHECK(!hasGameUpdate("3ds-old",""));
    auto cache=makeUpdateCache(source,1000,"3ds-new");CHECK(readUpdateCache(cache,source,1000,version));CHECK(version=="3ds-new");CHECK(readUpdateCache(cache,source,1899,version));CHECK(!readUpdateCache(cache,source,1900,version));CHECK(!readUpdateCache(cache,source,999,version));CHECK(!readUpdateCache(cache,other,1100,version));CHECK(!readUpdateCache(cache+"format=1\n",source,1100,version));CHECK(!readUpdateCache("garbage",source,1100,version));CHECK(makeUpdateCache(source,0,"v1").empty());CHECK(makeUpdateCache("http://unsafe",1000,"v1").empty());
    std::filesystem::create_directories("build-tests/check-cache");auto path="build-tests/check-cache/result.ini";std::filesystem::remove(path);
    std::atomic<int> calls{0};UpdateChecker worker([&](const std::string&,const std::function<bool()>&,std::string& result){++calls;result="3ds-new";return true;},[]{return 1000;});
    CHECK(worker.start(source,path));finish(worker);CHECK(calls==1);CHECK(worker.latestVersion()=="3ds-new");CHECK(worker.start(source,path));finish(worker);CHECK(calls==1);CHECK(worker.start(other,path));finish(worker);CHECK(calls==2);
    std::promise<void> entered,release;auto gate=release.get_future();
    UpdateChecker slow([&](const std::string&,const std::function<bool()>& cancelled,std::string& result){entered.set_value();gate.wait();if(cancelled())return false;result="3ds-stale";return true;},[]{return 2000;});
    CHECK(slow.start(source,"build-tests/check-cache/slow.ini"));CHECK(entered.get_future().wait_for(std::chrono::seconds(5))==std::future_status::ready);CHECK(slow.running());CHECK(slow.latestVersion().empty());CHECK(!slow.start(source,path));slow.cancel();release.set_value();finish(slow);CHECK(slow.latestVersion().empty());CHECK(!exists("build-tests/check-cache/slow.ini"));
    UpdateChecker offline([](const std::string&,const std::function<bool()>&,std::string&){return false;});CHECK(offline.start(source,"build-tests/check-cache/offline.ini"));finish(offline);CHECK(offline.latestVersion().empty());CHECK(!exists("build-tests/check-cache/offline.ini"));
    std::cout<<checks<<" background/cache checks passed.\n";
}
