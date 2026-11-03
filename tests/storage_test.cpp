#include "storage.h"
#include "sha256.h"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <vector>
namespace fs=std::filesystem;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"Échec ligne "<<__LINE__<<": "<<#x<<"\n";std::abort();}}while(0)
static std::string executable(char tag){std::string s(256,tag);s.replace(0,4,"3DSX");for(int i=4;i<32;i++)s[i]=0;s[4]=32;s[6]=8;s[16]=64;return s;}
static void put(const std::string&p,const std::string&s){CHECK(writeText(p,s));}
static std::string root;
static Storage fresh(const std::string&name){std::string dir=root+"/"+name;fs::remove_all(dir);fs::create_directories(dir);return Storage(dir);}
static void seed(Storage&s){put(s.game(),executable('A'));put(s.game()+".version","v1");put(fs::path(s.game()).parent_path().string()+"/pokesurvivor3ds.sav","SAVE-UNCHANGED");}
static void assertSave(Storage&s){CHECK(readText(fs::path(s.game()).parent_path().string()+"/pokesurvivor3ds.sav")=="SAVE-UNCHANGED");}
static void hashing(){
    Sha256 empty;CHECK(empty.finish()=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    Sha256 abc;abc.add("a",1);abc.add("bc",2);CHECK(abc.finish()=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    Sha256 big;std::string chunk(1000,'a');for(int i=0;i<1000;i++)big.add(chunk.data(),chunk.size());CHECK(big.finish()=="cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}
static void manifests(){
    std::string good="format=1\nversion=v1.2.3\nurl=https://github.com/test/test/releases/download/v1/game.3dsx\nsize=46000000\nsha256="+std::string(64,'a')+"\n";
    Release r;std::string e;CHECK(parseRelease(good,r,e));CHECK(r.size==46000000&&r.version=="v1.2.3");
    CHECK(!parseRelease(good+"version=v2\n",r,e));CHECK(!parseRelease(good+"unexpected=1\n",r,e));
    for(auto value:{"0","-1","12x","99999999999999999999","536870913"}){auto bad=good;bad.replace(bad.find("46000000"),8,value);CHECK(!parseRelease(bad,r,e));}
    auto bad=good;bad.replace(bad.find("https://"),8,"http://");CHECK(!parseRelease(bad,r,e));
    CHECK(!secureUrl("https://user:pass@github.com/file"));CHECK(!secureUrl("https://github.com\nHost: evil"));CHECK(!secureUrl("http://github.com"));CHECK(!secureUrl("https://github.com/#x"));
}
static void normal(){
    auto s=fresh("normal");seed(s);std::string e;
    put(s.staged(),executable('B'));CHECK(s.install("v2",e));CHECK(s.version()=="v2");CHECK(s.backupVersion()=="v1");CHECK(readText(s.game())==executable('B'));CHECK(readText(s.backup())==executable('A'));CHECK(!s.busy());assertSave(s);
    CHECK(s.restore(e,[](uint64_t,uint64_t){return true;}));CHECK(s.version()=="v1");CHECK(s.backupVersion()=="v2");CHECK(readText(s.game())==executable('A'));CHECK(readText(s.backup())==executable('B'));assertSave(s);
    CHECK(s.restore(e,[](uint64_t,uint64_t){return true;}));CHECK(s.version()=="v2");assertSave(s);
    CHECK(!s.restore(e,[](uint64_t,uint64_t){return false;}));CHECK(s.version()=="v2");CHECK(!exists(s.partial()));assertSave(s);
    auto corrupt=readText(s.backup());corrupt.back()='X';put(s.backup(),corrupt);CHECK(!s.restore(e,[](uint64_t,uint64_t){return true;}));CHECK(s.version()=="v2");CHECK(readText(s.game())==executable('B'));assertSave(s);
}
static void firstInstall(){auto s=fresh("first");std::string e;put(s.staged(),executable('A'));CHECK(s.install("v1",e));CHECK(s.version()=="v1");CHECK(!exists(s.backup()));CHECK(!s.restore(e,[](uint64_t,uint64_t){return true;}));}
static void invalid(){auto s=fresh("invalid");seed(s);std::string e;put(s.staged(),"3DSX bad file");CHECK(!s.install("v2",e));CHECK(!s.busy());CHECK(readText(s.game())==executable('A'));CHECK(!s.install("../invalid",e));assertSave(s);}
// Recreate all persistent boundaries of replacement and finalization. This is
// independent of install(): recovery must make the actual disk state usable.
static void interruption(){
    for(int first=0;first<2;first++)for(int phase=0;phase<8;phase++){
        auto s=fresh("crash"+std::to_string(first)+"-"+std::to_string(phase));std::string e;
        if(!first){seed(s);put(s.backup(),executable('Z'));put(s.backup()+".version","v0");}
        put(s.staged(),executable('B'));put(s.game()+".journal",std::string("v2\n")+(first?"inconnue":"v1"));
        if(phase>=1&&!first)CHECK(rename(s.game().c_str(),(s.game()+".previous").c_str())==0);
        if(phase>=2)CHECK(rename(s.staged().c_str(),s.game().c_str())==0);
        if(phase>=3)put(s.game()+".version","v2");
        if(phase>=4&&!first)CHECK(remove(s.backup().c_str())==0);
        if(phase>=5&&!first){std::string hash;uint64_t size;CHECK(hashFile(s.game()+".previous",hash,size));put(s.backup()+".sha256",hash);}
        if(phase>=6&&!first)put(s.backup()+".version","v1");
        if(phase>=7&&!first)CHECK(rename((s.game()+".previous").c_str(),s.backup().c_str())==0);
        CHECK(s.recover(e));CHECK(!s.busy());CHECK(!exists(s.game()+".previous"));CHECK(!exists(s.staged()));
        if(phase<2){CHECK(first?!exists(s.game()):readText(s.game())==executable('A'));}
        else{CHECK(readText(s.game())==executable('B'));CHECK(s.version()=="v2");if(!first){CHECK(readText(s.backup())==executable('A'));CHECK(s.backupVersion()=="v1");std::string hash;uint64_t size;CHECK(hashFile(s.backup(),hash,size));CHECK(hash==readText(s.backup()+".sha256"));}}
        if(!first)assertSave(s);
        CHECK(s.recover(e));
    }
    auto s=fresh("bad-journal");seed(s);put(s.game()+".journal","corrupt");std::string e;CHECK(!s.recover(e));CHECK(readText(s.game())==executable('A'));assertSave(s);
}
int main(int argc,char**argv){CHECK(argc==2);root=argv[1];hashing();manifests();normal();firstInstall();invalid();interruption();std::cout<<checks<<" vérifications réussies.\n";}
