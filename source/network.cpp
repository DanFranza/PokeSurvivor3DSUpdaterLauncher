#include "network.h"
#include "i18n.h"
#include "version.h"
#include <curl/curl.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <climits>
#include <memory>
#include <unistd.h>
#ifdef __3DS__
#include <3ds.h>
#include <malloc.h>
#endif

namespace {
uint64_t clockNs(){return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
long millis(uint64_t ns){return long(std::min<uint64_t>(LONG_MAX,std::max<uint64_t>(1,ns/1000000)));}
struct Environment {
#ifdef __3DS__
    u32* sockets=nullptr;
    bool soc=false,ssl=false;
#endif
    bool curl=false;
    bool open(std::string& error){
#ifdef __3DS__
        sockets=(u32*)memalign(0x1000,0x100000);
        if(!sockets){error="Initialisation Internet impossible";return false;}
        Result result=socInit(sockets,0x100000);
        if(R_FAILED(result)){error=detail("Initialisation Internet impossible",result);return false;}
        soc=true;result=sslcInit(0);
        if(R_FAILED(result)){error=detail("Initialisation Internet impossible",result);return false;}
        ssl=true;
#endif
        CURLcode resultCurl=curl_global_init(CURL_GLOBAL_DEFAULT);
        if(resultCurl!=CURLE_OK){error="Initialisation Internet impossible";return false;}
        curl=true;return true;
    }
#ifdef __3DS__
    static std::string detail(const char* stage,Result result){
        char code[32];snprintf(code,sizeof code," (0x%08lX).",(unsigned long)(uint32_t)result);
        return std::string(tr(stage))+code;
    }
#endif
    ~Environment(){
        if(curl)curl_global_cleanup();
#ifdef __3DS__
        if(ssl)sslcExit();
        if(soc)socExit();
        free(sockets);
#endif
    }
};
struct Transfer {
    FILE* output;
    CURL* curl;
    uint64_t limit,exact,total=0,lastUi=0;
    const Progress& progress;
    const FetchOptions& options;
    std::string& error;
    uint64_t started;
    uint64_t now()const{return options.nowNs?options.nowNs():clockNs();}
    bool stopped()const{return (options.cancelled&&options.cancelled())||(options.budgetNs&&now()-started>=options.budgetNs);}
    bool notify(bool force=false){
        if(stopped()){error="Téléchargement annulé.";return false;}
        uint64_t time=now();
        if(!force&&time-lastUi<100000000ull)return true;
        lastUi=time;
        curl_off_t expected=0;curl_easy_getinfo(curl,CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,&expected);
        if(!progress(total,exact?exact:(expected>0?uint64_t(expected):0))){error="Téléchargement annulé.";return false;}
        return true;
    }
    static size_t write(char* data,size_t size,size_t count,void* pointer){
        auto& t=*static_cast<Transfer*>(pointer);
        if(size&&count>SIZE_MAX/size)return 0;
        size_t bytes=size*count;
        long status=0;curl_easy_getinfo(t.curl,CURLINFO_RESPONSE_CODE,&status);
        // Redirect/error response bodies never enter the staged game file.
        if(status!=200)return bytes;
        if(t.stopped()){t.error="Téléchargement annulé.";return 0;}
        if(bytes>t.limit-t.total){t.error="Téléchargement plus grand que prévu.";return 0;}
        if(fwrite(data,1,bytes,t.output)!=bytes){t.error="Carte SD pleine ou inaccessible.";return 0;}
        t.total+=bytes;
        return bytes;
    }
    static int update(void* pointer,curl_off_t,curl_off_t,curl_off_t,curl_off_t){
        return static_cast<Transfer*>(pointer)->stopped()?1:0;
    }
};
}

bool fetch(const std::string& initial,const std::string& path,uint64_t limit,uint64_t exact,std::string& error,const Progress& progress,const FetchOptions& options){
    error.clear();
    if(!secureUrl(initial)){error="Adresse HTTPS invalide ou redirection non sûre.";return false;}
    uint64_t started=options.nowNs?options.nowNs():clockNs();
    auto stopped=[&]{uint64_t now=options.nowNs?options.nowNs():clockNs();return (options.cancelled&&options.cancelled())||(options.budgetNs&&now-started>=options.budgetNs);};
    if(stopped()){error="Téléchargement annulé.";return false;}
    Environment environment;if(!environment.open(error))return false;
    std::string url=initial;bool ok=false;
    for(int redirects=0;redirects<=8;++redirects){
        if(stopped()){error="Téléchargement annulé.";break;}
        if(!secureUrl(url)){error="Adresse HTTPS invalide ou redirection non sûre.";break;}
        CURL* curl=curl_easy_init();CURLM* multi=curl_multi_init();
        if(!curl||!multi){if(curl)curl_easy_cleanup(curl);if(multi)curl_multi_cleanup(multi);error="Initialisation Internet impossible";break;}
        FILE* output=fopen(path.c_str(),"wb");
        if(!output){curl_easy_cleanup(curl);curl_multi_cleanup(multi);error="Impossible d'écrire sur la carte SD.";break;}
        Transfer transfer{output,curl,limit,exact,0,0,progress,options,error,started};
        char diagnostic[CURL_ERROR_SIZE]{};
        CURLcode setting=CURLE_OK;
        auto set=[&](CURLoption key,auto value){CURLcode result=curl_easy_setopt(curl,key,value);if(setting==CURLE_OK)setting=result;};
        set(CURLOPT_ERRORBUFFER,diagnostic);
        set(CURLOPT_URL,url.c_str());
        set(CURLOPT_USERAGENT,"PokeSurvivor-Launcher/" LAUNCHER_VERSION);
        set(CURLOPT_CAINFO,"romfs:/roots.pem");
        set(CURLOPT_SSL_VERIFYPEER,1L);set(CURLOPT_SSL_VERIFYHOST,2L);
        set(CURLOPT_SSLVERSION,long(CURL_SSLVERSION_TLSv1_2));
        set(CURLOPT_PROTOCOLS_STR,"https");set(CURLOPT_REDIR_PROTOCOLS_STR,"https");
        set(CURLOPT_FOLLOWLOCATION,0L);set(CURLOPT_NOSIGNAL,1L);
#ifdef NETWORK_TESTING
        set(CURLOPT_PROXY,"");
#endif
        set(CURLOPT_CONNECTTIMEOUT_MS,millis(options.timeoutNs));
        set(CURLOPT_LOW_SPEED_LIMIT,1L);set(CURLOPT_LOW_SPEED_TIME,std::max(1L,millis(options.timeoutNs)/1000));
        set(CURLOPT_WRITEFUNCTION,&Transfer::write);set(CURLOPT_WRITEDATA,&transfer);
        set(CURLOPT_XFERINFOFUNCTION,&Transfer::update);set(CURLOPT_XFERINFODATA,&transfer);set(CURLOPT_NOPROGRESS,0L);
        set(CURLOPT_HTTP_VERSION,long(CURL_HTTP_VERSION_1_1));
        if(options.budgetNs){uint64_t elapsed=(options.nowNs?options.nowNs():clockNs())-started;set(CURLOPT_TIMEOUT_MS,millis(elapsed<options.budgetNs?options.budgetNs-elapsed:1));}
#ifdef NETWORK_TESTING
        struct curl_slist* mapping=nullptr;
        if(const char* route=getenv("LAUNCHER_TEST_ROUTE")){mapping=curl_slist_append(mapping,route);set(CURLOPT_CONNECT_TO,mapping);}
#endif
        CURLcode result=setting;CURLMcode multiResult=CURLM_OK;bool added=false;
        if(result==CURLE_OK&&transfer.notify(true)){
            multiResult=curl_multi_add_handle(multi,curl);added=multiResult==CURLM_OK;
            int running=0;
            while(added&&multiResult==CURLM_OK){
                multiResult=curl_multi_perform(multi,&running);
                if(!transfer.notify()){result=CURLE_ABORTED_BY_CALLBACK;break;}
                if(multiResult!=CURLM_OK||!running)break;
                int ready=0;
                long wait=options.pollNs?std::min(100L,millis(options.pollNs)):100L;
                multiResult=curl_multi_wait(multi,nullptr,0,int(wait),&ready);
            }
            int remaining=0;bool completed=false;
            while(CURLMsg* message=curl_multi_info_read(multi,&remaining)){
                if(message->msg==CURLMSG_DONE){if(result==CURLE_OK)result=message->data.result;completed=true;}
            }
            if(result==CURLE_OK&&(!completed||multiResult!=CURLM_OK))result=CURLE_RECV_ERROR;
        }else if(result==CURLE_OK)result=CURLE_ABORTED_BY_CALLBACK;
        long status=0;curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&status);
        char* location=nullptr;curl_easy_getinfo(curl,CURLINFO_REDIRECT_URL,&location);
        std::string next=location?location:"";
#ifdef __3DS__
        if(result!=CURLE_OK){
            long osError=0;curl_easy_getinfo(curl,CURLINFO_OS_ERRNO,&osError);
            FILE* log=fopen("sdmc:/3ds/PokeSurvivorLauncher/network-error.txt","w");
            if(log){fprintf(log,"curl %d, network %ld: %s\n",int(result),osError,diagnostic);fclose(log);}
        }
#endif
        if(added)curl_multi_remove_handle(multi,curl);
        curl_easy_cleanup(curl);curl_multi_cleanup(multi);
#ifdef NETWORK_TESTING
        curl_slist_free_all(mapping);
#endif
        bool written=fflush(output)==0&&fsync(fileno(output))==0;
        if(fclose(output))written=false;
        if(result!=CURLE_OK){
            if(error.empty()){
                if(result==CURLE_PEER_FAILED_VERIFICATION)error=tr("Certificat HTTPS refusé. Vérifiez la date et l'heure de la console.");
                else if(result==CURLE_SSL_CACERT_BADFILE)error=tr("Impossible de charger les certificats HTTPS.");
                else error=trf("Connexion HTTPS impossible (curl {0}).",{std::to_string(result)});
            }
            break;
        }
        if(status==301||status==302||status==303||status==307||status==308){
            if(redirects==8){error="Trop de redirections HTTPS.";break;}
            if(next.empty()||!secureUrl(next)){error="Adresse HTTPS invalide ou redirection non sûre.";break;}
            url=next;continue;
        }
        if(status!=200){error=trf("Serveur HTTP : {0}. Vérifier le fichier publié sur GitHub.",{std::to_string(status)});break;}
        if(exact&&transfer.total!=exact){error="Téléchargement incomplet.";break;}
        if(!written){error="Erreur d'écriture sur la carte SD.";break;}
        ok=true;break;
    }
    if(!ok)remove(path.c_str());
    return ok;
}
