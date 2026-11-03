#include "cia.h"
#include "game_format.h"
#include "storage.h"
#include <3ds.h>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <cstring>
static unsigned checks=0,starts=0,overwrites=0,finishes=0,cancels=0,exits=0,writes=0;
static bool installed=false,writeFails=false,finishFails=false;static u64 imported=0,launched=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"CIA test failed at "<<__LINE__<<": "<<#x<<"\n";std::abort();}}while(0)
Result amInit(){return 0;}void amExit(){++exits;}
Result AM_GetTitleInfo(int media,u32 count,u64* id,AM_TitleInfo*){CHECK(media==MEDIATYPE_SD&&count==1&&*id==GAME_CIA_TITLE_ID);return installed?0:-1;}
Result AM_StartCiaInstall(int media,Handle* handle){CHECK(media==MEDIATYPE_SD);++starts;*handle=42;imported=0;return 0;}
Result AM_StartCiaInstallOverwrite(Handle* handle,int media){CHECK(media==MEDIATYPE_SD);++overwrites;*handle=42;imported=0;return 0;}
Result AM_FinishCiaInstall(Handle h){CHECK(h==42);++finishes;if(finishFails)return -1;installed=true;return 0;}
Result AM_CancelCIAInstall(Handle h){CHECK(h==42);++cancels;return 0;}
Result FSFILE_Write(Handle h,u32* count,u64 at,const void*,u32 size,u32 flags){CHECK(h==42&&flags==0&&at==imported);++writes;if(writeFails)return -1;*count=size;imported+=size;return 0;}
void aptSetChainloader(u64 id,uint8_t media){CHECK(id==GAME_CIA_TITLE_ID&&media==MEDIATYPE_SD);launched=id;}
static std::string cia(u64 id=GAME_CIA_TITLE_ID){
    const size_t tmd=0x2040,content=0x2340,length=70000;std::string data(content+length,0);
    auto le=[&](size_t at,u64 n,int bytes){for(int i=0;i<bytes;i++)data[at+i]=n>>(i*8);};
    auto be=[&](size_t at,u64 n,int bytes){for(int i=0;i<bytes;i++)data[at+bytes-1-i]=n>>(i*8);};
    le(0,0x2020,4);le(16,0x300,4);le(24,length,8);be(tmd,0x10004,4);be(tmd+0x140+0x4c,id,8);data.replace(content+0x100,4,"NCCH");le(content+0x118,id,8);return data;
}
int main(int argc,char** argv){
    if(argc==2){CHECK(validGameCia(argv[1]));std::cout<<"Published game CIA identity verified.\n";return 0;}
    std::filesystem::create_directories("build-tests/cia");auto path="build-tests/cia/game.cia";CHECK(writeText(path,cia()));CHECK(validGameCia(path));
    std::string error;CHECK(installGameCia(path,error,[](uint64_t,uint64_t){return true;}));CHECK(starts==1&&overwrites==0&&finishes==1&&cancels==0);CHECK(imported==cia().size());CHECK(gameCiaInstalled());CHECK(prepareGameCiaLaunch(error));CHECK(launched==GAME_CIA_TITLE_ID);
    CHECK(installGameCia(path,error,[](uint64_t,uint64_t){return true;}));CHECK(overwrites==1);
    CHECK(!installGameCia(path,error,[](uint64_t,uint64_t){return false;}));CHECK(cancels==1&&finishes==2);
    writeFails=true;CHECK(!installGameCia(path,error,[](uint64_t,uint64_t){return true;}));CHECK(cancels==2);writeFails=false;
    finishFails=true;CHECK(!installGameCia(path,error,[](uint64_t,uint64_t){return true;}));CHECK(cancels==3);finishFails=false;
    auto oldStarts=starts,oldWrites=writes;CHECK(writeText(path,cia(GAME_CIA_TITLE_ID-0x100)));CHECK(!validGameCia(path));CHECK(!installGameCia(path,error,[](uint64_t,uint64_t){return true;}));CHECK(starts==oldStarts&&writes==oldWrites);
    auto truncated=cia();truncated.resize(truncated.size()-1);CHECK(writeText(path,truncated));CHECK(!validGameCia(path));
    std::filesystem::remove_all("build-tests/cia/managed");std::filesystem::create_directories("build-tests/cia/managed");
    Storage storage("build-tests/cia/managed","game.cia",validGameCia);auto a=cia(),b=cia(),c=cia();a.back()='A';b.back()='B';c.back()='C';
    CHECK(writeText("build-tests/cia/managed/pokesurvivor3ds.sav","KEEP-SAVE"));
    auto importer=[](const std::string& file,std::string& e){return installGameCia(file,e,[](uint64_t,uint64_t){return true;});};
    CHECK(writeText(storage.staged(),a));CHECK(storage.install("cia-A",error,importer));CHECK(storage.version()=="cia-A");
    CHECK(writeText(storage.staged(),b));CHECK(storage.install("cia-B",error,importer));CHECK(storage.backupVersion()=="cia-A");
    CHECK(storage.restore(error,[](uint64_t,uint64_t){return true;},importer));CHECK(storage.version()=="cia-A");CHECK(storage.backupVersion()=="cia-B");
    CHECK(writeText(storage.staged(),c));CHECK(!storage.install("cia-C",error,[](const std::string&,std::string&){return false;}));CHECK(storage.busy());CHECK(storage.version()=="cia-A");
    CHECK(storage.recover(error,importer));CHECK(storage.version()=="cia-C");CHECK(storage.backupVersion()=="cia-A");CHECK(!storage.busy());
    CHECK(writeText(storage.staged(),b));CHECK(!storage.install("cia-B",error,[](const std::string&,std::string&){return false;}));CHECK(storage.recover(error));CHECK(storage.version()=="cia-C");CHECK(!exists(storage.staged()));
    CHECK(readText("build-tests/cia/managed/pokesurvivor3ds.sav")=="KEEP-SAVE");
    CHECK(exits>0);std::cout<<checks<<" CIA installation checks passed.\n";
}
