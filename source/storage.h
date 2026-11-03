#pragma once
#include <string>
#include <functional>
#include <cstdint>
#include "game_format.h"
struct Release {std::string version,url,sha256; uint64_t size=0; std::string gitSha1{}; GameFormat format=GameFormat::ThreeDSX;};
bool parseRelease(const std::string&,Release&,std::string&);
bool secureUrl(const std::string&);
std::string readText(const std::string&);
bool exists(const std::string&);
bool writeText(const std::string&,const std::string&);
bool valid3dsx(const std::string&);
bool hashFile(const std::string&,std::string&,uint64_t&);
class Storage {
    std::string base,fileName;
    std::function<bool(const std::string&)> valid_;
    bool commitPrepared(std::string&);
    bool finish(const std::string&,std::string&);
public:
    using Import=std::function<bool(const std::string&,std::string&)>;
    explicit Storage(std::string path,std::string name="pokesurvivor3ds.3dsx",std::function<bool(const std::string&)> valid=valid3dsx):base(path),fileName(name),valid_(valid){}
    std::string game()const{return base+"/"+fileName;}
    std::string backup()const{return game()+".bak";}
    std::string staged()const{return game()+".new";}
    std::string partial()const{return game()+".part";}
    std::string version()const;
    std::string backupVersion()const;
    bool recover(std::string&,const Import& import={});
    bool install(const std::string& version,std::string&,const Import& import={});
    bool restore(std::string&,const std::function<bool(uint64_t,uint64_t)>& progress,const Import& import={});
    bool hasBackupFiles()const{return exists(backup())||exists(backup()+".version")||exists(backup()+".sha256");}
    bool deleteBackup(std::string&);
    bool busy()const{return exists(game()+".journal");}
};
