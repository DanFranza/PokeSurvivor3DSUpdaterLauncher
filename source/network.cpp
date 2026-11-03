#include "network.h"
#include "i18n.h"
#include "version.h"
#include <3ds.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <algorithm>

static std::string failure(const char* stage,Result r){char b[120];snprintf(b,sizeof b,"%s (0x%08lX).",tr(stage),(unsigned long)r);return b;}
static bool trust(httpcContext*c,const std::function<bool()>& stopped){
    FILE*f=fopen("romfs:/roots.bin","rb");if(!f)return false;
    bool ok=true;unsigned char sz[4];
    while(fread(sz,1,4,f)==4){if(stopped()){ok=false;break;}uint32_t n=sz[0]|sz[1]<<8|sz[2]<<16|sz[3]<<24;if(n>8192||!n){ok=false;break;}std::vector<u8>b(n);if(fread(b.data(),1,n,f)!=n||R_FAILED(httpcAddTrustedRootCA(c,b.data(),n))){ok=false;break;}}
    ok=ok&&!ferror(f);fclose(f);return ok;
}
bool fetch(const std::string&initial,const std::string&path,uint64_t limit,uint64_t exact,std::string&e,const Progress&progress,const FetchOptions&options){
    auto now=[&](){return options.nowNs?options.nowNs():uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());};
    const uint64_t started=now();
    auto stopped=[&](){return (options.cancelled&&options.cancelled())||(options.budgetNs&&now()-started>=options.budgetNs);};
    auto waitTime=[&](){uint64_t n=options.timeoutNs;if(options.pollNs)n=std::min(n,options.pollNs);if(options.budgetNs){uint64_t elapsed=now()-started;n=std::min(n,elapsed<options.budgetNs?options.budgetNs-elapsed:0);}return n;};
    if(stopped()){e="Téléchargement annulé.";return false;}
    Result r=httpcInit(0);if(R_FAILED(r)){e=failure("Initialisation Internet impossible",r);return false;}
    std::string url=initial;bool ok=false;FILE*out=nullptr;
    for(int redirects=0;redirects<=8;redirects++){
        if(stopped()){e="Téléchargement annulé.";break;}
        if(!secureUrl(url)){e="Adresse HTTPS invalide ou redirection non sûre.";break;}
        httpcContext c{};r=httpcOpenContext(&c,HTTPC_METHOD_GET,url.c_str(),1);
        if(R_FAILED(r)){e=failure("Connexion HTTPS impossible",r);break;}
        auto close=[&](){httpcCloseContext(&c);};
        if(!trust(&c,stopped)){e="Impossible de charger les certificats HTTPS.";close();break;}
        r=httpcAddRequestHeaderField(&c,"User-Agent","PokeSurvivor-Launcher/" LAUNCHER_VERSION);
        if(R_SUCCEEDED(r))r=httpcAddRequestHeaderField(&c,"Accept-Encoding","identity");
        if(R_SUCCEEDED(r))r=httpcBeginRequest(&c);
        u32 status=0;
        if(R_SUCCEEDED(r)){
            do{
                if(stopped()){r=Result(HTTPC_RESULTCODE_TIMEDOUT);break;}
                r=httpcGetResponseStatusCodeTimeout(&c,&status,waitTime());
            }while((u32)r==HTTPC_RESULTCODE_TIMEDOUT&&options.pollNs&&!stopped());
        }
        if(R_FAILED(r)){e=failure("Connexion refusée ou délai dépassé",r);close();break;}
        if(status==301||status==302||status==303||status==307||status==308){
            char loc[2048]{};r=httpcGetResponseHeader(&c,"Location",loc,sizeof loc);close();
            if(R_FAILED(r)||!memchr(loc,0,sizeof loc)){e="Redirection GitHub illisible.";break;}
            std::string next=loc;
            if(next.compare(0,2,"//")==0)next="https:"+next;
            else if(!next.empty()&&next[0]=='/'){auto slash=url.find('/',8);next=url.substr(0,slash)+next;}
            url=next;if(redirects==8)e="Trop de redirections HTTPS.";continue;
        }
        if(status!=200){e=trf("Serveur HTTP : {0}. Vérifier le fichier publié sur GitHub.",{std::to_string(status)});close();break;}
        out=fopen(path.c_str(),"wb");if(!out){e="Impossible d'écrire sur la carte SD.";close();break;}
        std::vector<u8>buffer(32768);uint64_t total=0;bool body=true;
        while(true){
            if(stopped()){e="Téléchargement annulé.";body=false;break;}
            u32 before=0,after=0,length=0;
            r=httpcGetDownloadSizeState(&c,&before,&length);if(R_FAILED(r)){e=failure("Lecture du transfert impossible",r);body=false;break;}
            r=httpcReceiveDataTimeout(&c,buffer.data(),buffer.size(),waitTime());
            Result sr=httpcGetDownloadSizeState(&c,&after,&length);
            if(R_FAILED(sr)||after<before||after-before>buffer.size()){e="Compteur de téléchargement invalide.";body=false;break;}
            size_t n=after-before;
            if(total+n>limit){e="Téléchargement plus grand que prévu.";body=false;break;}
            if(n&&fwrite(buffer.data(),1,n,out)!=n){e="Carte SD pleine ou inaccessible.";body=false;break;}
            total+=n;
            if(!progress(total,exact?exact:length)){e="Téléchargement annulé.";body=false;break;}
            if((u32)r==HTTPC_RESULTCODE_TIMEDOUT&&options.pollNs)continue;
            if(R_FAILED(r)&&(u32)r!=HTTPC_RESULTCODE_DOWNLOADPENDING){e=failure("Téléchargement interrompu",r);body=false;break;}
            if(R_SUCCEEDED(r))break;
            if(!n&&options.pollNs)continue;
            if(!n){e="Transfert interrompu sans données.";body=false;break;}
        }
        close();
        if(fflush(out)||fsync(fileno(out)))body=false;
        if(fclose(out))body=false;
        out=nullptr;
        if(exact&&total!=exact){e="Téléchargement incomplet.";body=false;}
        if(!body&&e.empty())e="Erreur d'écriture sur la carte SD.";
        ok=body;break;
    }
    if(out)fclose(out);
    httpcExit();
    if(!ok)remove(path.c_str());
    return ok;
}
