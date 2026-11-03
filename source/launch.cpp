#include "launch.h"
#include "i18n.h"
#include <3ds.h>
#include <cstring>
#include <cstdio>
// Luma hb:ldr IPC: SetTarget=2, SetArgv=3. The installed CIA chains to
// Luma's configured homebrew title; a 3DSX returns through hbmenu's loader.
bool prepareLaunch(const std::string&path,std::string&e){
    u64 tid=0;
    if(!envIsHomebrew()){
        s64 configured=0;Result r=svcGetSystemInfo(&configured,0x10000,0x100);
        if(R_FAILED(r)){e="Luma3DS avec chargeur homebrew est requis.";return false;}
        tid=configured?u64(configured):0x000400000d921e00ull;
        r=amInit();if(R_FAILED(r)){e="Impossible de vérifier Homebrew Launcher.";return false;}
        AM_TitleInfo entry{};r=AM_GetTitleInfo(MEDIATYPE_SD,1,&tid,&entry);amExit();
        if(R_FAILED(r)){e="Installer Homebrew Launcher Loader (CIA), ou vérifier le titre homebrew configuré dans Luma.";return false;}
    }
    Handle h=0;Result r=svcConnectToPort(&h,"hb:ldr");
    if(R_FAILED(r)){e="Chargeur Luma3DS hb:ldr indisponible.";return false;}
    std::string target=path.compare(0,5,"sdmc:")==0?path.substr(5):path;
    if(target.size()+1>1024){svcCloseHandle(h);e="Chemin du jeu trop long.";return false;}
    u32*cmd=getThreadCommandBuffer();cmd[0]=IPC_MakeHeader(2,0,2);cmd[1]=IPC_Desc_StaticBuffer(target.size()+1,0);cmd[2]=(u32)target.c_str();
    r=svcSendSyncRequest(h);if(R_SUCCEEDED(r))r=cmd[1];
    if(R_SUCCEEDED(r)){
        alignas(4) char args[1024]{};*(u32*)args=1;strcpy(args+4,path.c_str());
        cmd[0]=IPC_MakeHeader(3,0,2);cmd[1]=IPC_Desc_StaticBuffer(sizeof args,1);cmd[2]=(u32)args;
        r=svcSendSyncRequest(h);if(R_SUCCEEDED(r))r=cmd[1];
    }
    svcCloseHandle(h);
    if(R_FAILED(r)){char b[96];snprintf(b,sizeof b,tr("Lancement refusé (0x%08lX)."),(unsigned long)r);e=b;return false;}
    if(tid)aptSetChainloader(tid,MEDIATYPE_SD);
    return true;
}
