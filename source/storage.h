#pragma once
#include <string>
#include <functional>
#include <cstdint>
struct Release {std::string version,url,sha256; uint64_t size=0; std::string gitSha1{};};
bool parseRelease(const std::string&,Release&,std::string&);
bool secureUrl(const std::string&);
std::string readText(const std::string&);
bool exists(const std::string&);
bool writeText(const std::string&,const std::string&);
bool valid3dsx(const std::string&);
bool hashFile(const std::string&,std::string&,uint64_t&);
class Storage {
    std::string base;
    bool finish(const std::string&,std::string&);
public:
    explicit Storage(std::string path):base(path){}
    std::string game()const{return base+"/pokesurvivor3ds.3dsx";}
    std::string backup()const{return game()+".bak";}
    std::string staged()const{return game()+".new";}
    std::string partial()const{return game()+".part";}
    std::string version()const;
    std::string backupVersion()const;
    bool recover(std::string&);
    bool install(const std::string& version,std::string&);
    bool restore(std::string&,const std::function<bool(uint64_t,uint64_t)>& progress);
    bool busy()const{return exists(game()+".journal");}
};
