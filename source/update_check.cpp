#include "update_check.h"
#include "storage.h"
#include <algorithm>
#include <ctime>
#include <map>
#include <cstdlib>

namespace {
bool validVersion(const std::string& s){return !s.empty()&&s.size()<=48&&std::all_of(s.begin(),s.end(),[](unsigned char c){return(c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='.'||c=='-'||c=='_';});}
constexpr int64_t CACHE_SECONDS=15*60;
}
bool hasGameUpdate(const std::string& installed,const std::string& available){return validVersion(installed)&&installed!="inconnue"&&validVersion(available)&&installed!=available;}
bool readUpdateCache(const std::string& text,const std::string& source,int64_t now,std::string& version){
    std::map<std::string,std::string> fields;size_t at=0;
    while(at<text.size()){auto end=text.find('\n',at);if(end==std::string::npos)end=text.size();auto line=text.substr(at,end-at);at=end+1;auto equal=line.find('=');if(equal==std::string::npos||!fields.emplace(line.substr(0,equal),line.substr(equal+1)).second)return false;}
    if(fields.size()!=4||fields["format"]!="1"||fields["source"]!=source||!validVersion(fields["version"]))return false;
    auto stamp=fields["checked"];if(stamp.empty()||stamp.size()>12||!std::all_of(stamp.begin(),stamp.end(),[](char c){return c>='0'&&c<='9';}))return false;
    int64_t checked=strtoll(stamp.c_str(),nullptr,10);
    if(checked<=0||now<checked||now-checked>=CACHE_SECONDS)return false;
    version=fields["version"];return true;
}
std::string makeUpdateCache(const std::string& source,int64_t now,const std::string& version){
    if(!secureUrl(source)||now<=0||!validVersion(version))return {};
    return "format=1\nsource="+source+"\nchecked="+std::to_string(now)+"\nversion="+version+"\n";
}
UpdateChecker::UpdateChecker(Query query,std::function<int64_t()> clock):query_(std::move(query)),clock_(std::move(clock)){}
UpdateChecker::~UpdateChecker(){cancel();
#ifdef __3DS__
    if(worker_){threadJoin(worker_,U64_MAX);threadFree(worker_);}
#else
    if(worker_.joinable())worker_.join();
#endif
}
bool UpdateChecker::start(const std::string& source,const std::string& cache){
    if(running())return false;
    collect();source_=source;cache_=cache;latest_.clear();cancel_.store(false);done_.store(false);
#ifdef __3DS__
    s32 priority=0x30;svcGetThreadPriority(&priority,CUR_THREAD_HANDLE);
    worker_=threadCreate(entry,this,128*1024,std::min(0x3f,int(priority)+2),-2,false);
    if(!worker_){done_.store(true);return false;}
#else
    try{worker_=std::thread([this]{run();});}catch(...){done_.store(true);return false;}
#endif
    return true;
}
void UpdateChecker::cancel(){cancel_.store(true);}
bool UpdateChecker::running()const{return !done_.load(std::memory_order_acquire);}
void UpdateChecker::collect(){
    if(running())return;
#ifdef __3DS__
    if(worker_){threadJoin(worker_,U64_MAX);threadFree(worker_);worker_=nullptr;}
#else
    if(worker_.joinable())worker_.join();
#endif
}
void UpdateChecker::clearResult(){if(!running()){collect();latest_.clear();}}
std::string UpdateChecker::latestVersion()const{return running()?std::string():latest_;}
void UpdateChecker::entry(void* pointer){static_cast<UpdateChecker*>(pointer)->run();}
void UpdateChecker::run(){
    try{
        int64_t now=clock_?clock_():std::time(nullptr);std::string available;
        if(!cancel_.load()&&secureUrl(source_)){
            if(readUpdateCache(readText(cache_),source_,now,available))latest_=available;
            else if(query_(source_,[this]{return cancel_.load();},available)&&!cancel_.load()&&validVersion(available)){
                auto cache=makeUpdateCache(source_,now,available);if(!cache.empty())writeText(cache_,cache);latest_=available;
            }
        }
    }catch(...){latest_.clear();}
    done_.store(true,std::memory_order_release);
}
