#include "cia.h"
#include "game_format.h"
#include "i18n.h"
#include <3ds.h>
#include <cstdio>
#include <vector>
#include <sys/stat.h>

namespace {
std::string failed(const char* stage,Result result){char text[200];snprintf(text,sizeof text,"%s (0x%08lX).",tr(stage),(unsigned long)result);return text;}
}
bool gameCiaInstalled(){
    Result result=amInit();if(R_FAILED(result))return false;
    u64 id=GAME_CIA_TITLE_ID;AM_TitleInfo info{};result=AM_GetTitleInfo(MEDIATYPE_SD,1,&id,&info);amExit();return R_SUCCEEDED(result);
}
bool installGameCia(const std::string& path,std::string& error,const Progress& progress){
    if(!validGameCia(path)){error="CIA du jeu invalide ou titre incorrect.";return false;}
    FILE* input=fopen(path.c_str(),"rb");if(!input){error="Lecture du CIA impossible.";return false;}
    struct stat info{};if(fstat(fileno(input),&info)){fclose(input);error="Lecture du CIA impossible.";return false;}
    Result result=amInit();if(R_FAILED(result)){fclose(input);error=failed("Service d'installation CIA indisponible",result);return false;}
    u64 id=GAME_CIA_TITLE_ID;AM_TitleInfo installed{};bool exists=R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_SD,1,&id,&installed));
    Handle handle=0;result=exists?AM_StartCiaInstallOverwrite(&handle,MEDIATYPE_SD):AM_StartCiaInstall(MEDIATYPE_SD,&handle);
    if(R_FAILED(result)||!handle){amExit();fclose(input);error=failed("Installation CIA impossible",result);return false;}
    std::vector<unsigned char> buffer(32768);uint64_t offset=0;size_t n;bool ok=true;
    while((n=fread(buffer.data(),1,buffer.size(),input))){
        u32 written=0;result=FSFILE_Write(handle,&written,offset,buffer.data(),n,0);
        if(R_FAILED(result)||written!=n){error=failed("Écriture du CIA impossible",result);ok=false;break;}
        offset+=n;
        if(!progress(offset,info.st_size)){error="Installation CIA annulée.";ok=false;break;}
    }
    if(ferror(input)||offset!=uint64_t(info.st_size)){if(error.empty())error="Lecture du CIA impossible.";ok=false;}
    fclose(input);
    if(ok){result=AM_FinishCiaInstall(handle);if(R_FAILED(result)){error=failed("Finalisation CIA impossible",result);AM_CancelCIAInstall(handle);ok=false;}}
    else AM_CancelCIAInstall(handle);
    amExit();return ok;
}
bool prepareGameCiaLaunch(std::string& error){
    if(!gameCiaInstalled()){error="Le jeu CIA n'est pas installé. Utilisez Mettre à jour.";return false;}
    aptSetChainloader(GAME_CIA_TITLE_ID,MEDIATYPE_SD);return true;
}
