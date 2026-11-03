#include "storage.h"
#include "sha256.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <map>
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>

bool exists(const std::string&p){struct stat s;return stat(p.c_str(),&s)==0;}
std::string readText(const std::string&p){FILE*f=fopen(p.c_str(),"rb");if(!f)return {};std::vector<char> b(8193);size_t n=fread(b.data(),1,b.size(),f);bool ok=!ferror(f)&&n<=8192;fclose(f);return ok?std::string(b.data(),n):std::string();}
static bool closeWrite(FILE*f){bool ok=fflush(f)==0;if(ok)ok=fsync(fileno(f))==0;return fclose(f)==0&&ok;}
bool writeText(const std::string&p,const std::string&s){std::string t=p+".tmp";FILE*f=fopen(t.c_str(),"wb");if(!f)return false;bool ok=fwrite(s.data(),1,s.size(),f)==s.size();ok=closeWrite(f)&&ok;if(!ok){remove(t.c_str());return false;}if(exists(p)&&remove(p.c_str()))return false;return rename(t.c_str(),p.c_str())==0;}
bool secureUrl(const std::string&u){if(u.size()>2000||u.compare(0,8,"https://"))return false;auto end=u.find('/',8);std::string host=u.substr(8,end==std::string::npos?end:end-8);if(host.empty()||host.find_first_of("@:#?\\")!=std::string::npos)return false;for(unsigned char c:u)if(c<=32||c>=127)return false;return u.find('#')==std::string::npos;}
static bool versionOk(const std::string&s){return !s.empty()&&s.size()<=48&&std::all_of(s.begin(),s.end(),[](unsigned char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='.'||c=='-'||c=='_';});}
bool parseRelease(const std::string&s,Release&r,std::string&e){
    std::map<std::string,std::string> m;size_t at=0;
    while(at<s.size()){auto end=s.find('\n',at);if(end==std::string::npos)end=s.size();auto line=s.substr(at,end-at);at=end+1;if(!line.empty()&&line.back()=='\r')line.pop_back();if(line.empty()||line[0]=='#')continue;auto eq=line.find('=');if(eq==std::string::npos||!m.emplace(line.substr(0,eq),line.substr(eq+1)).second){e="Manifeste invalide (ligne ou doublon).";return false;}}
    if(m.size()!=5||m["format"]!="1"||!versionOk(m["version"])||!secureUrl(m["url"])||m["sha256"].size()!=64||!std::all_of(m["sha256"].begin(),m["sha256"].end(),[](char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f');})){e="Manifeste de mise à jour invalide.";return false;}
    auto n=m["size"];if(n.empty()||n.size()>10||!std::all_of(n.begin(),n.end(),[](char c){return c>='0'&&c<='9';})){e="Taille du jeu invalide.";return false;}
    uint64_t size=strtoull(n.c_str(),nullptr,10);if(size<32||size>512ull*1024*1024){e="Taille du jeu hors limites.";return false;}
    r={m["version"],m["url"],m["sha256"],size};return true;
}
bool valid3dsx(const std::string&p){
    FILE*f=fopen(p.c_str(),"rb");if(!f)return false;
    unsigned char h[44]{};size_t count=fread(h,1,sizeof h,f);
    bool ok=count>=32&&memcmp(h,"3DSX",4)==0;
    auto u16=[&](int at){return uint32_t(h[at])|(uint32_t(h[at+1])<<8);};
    auto u32=[&](int at){return u16(at)|(u16(at+2)<<16);};
    struct stat st{};ok=ok&&fstat(fileno(f),&st)==0;
    uint32_t header=u16(4),reloc=u16(6),code=u32(16),ro=u32(20),data=u32(24),bss=u32(28);
    ok=ok&&(header==32||header==44)&&reloc>=8&&reloc<=64&&code>0&&bss<=data;
    uint64_t minimum=uint64_t(header)+3*reloc+code+ro+data-bss;
    ok=ok&&minimum<=uint64_t(st.st_size);
    if(header==44){uint64_t smdh=u32(32),smdhSize=u32(36),romfs=u32(40);ok=ok&&smdh+smdhSize<=uint64_t(st.st_size)&&romfs<=uint64_t(st.st_size);}
    fclose(f);return ok;
}
bool hashFile(const std::string&p,std::string&hash,uint64_t&size){FILE*f=fopen(p.c_str(),"rb");if(!f)return false;Sha256 sha;std::vector<char> b(32768);size=0;size_t n;while((n=fread(b.data(),1,b.size(),f))){sha.add(b.data(),n);size+=n;}bool ok=!ferror(f);fclose(f);hash=sha.finish();return ok;}
std::string Storage::version()const{auto s=readText(game()+".version");return versionOk(s)?s:"inconnue";}
std::string Storage::backupVersion()const{auto s=readText(backup()+".version");return versionOk(s)?s:"inconnue";}
// The journal is written before any rename. A remaining .new means the
// replacement has not committed: recovery returns to the original executable.
// Without .new, finish the committed replacement, keeping .previous as backup.
bool Storage::finish(const std::string&j,std::string&e){
    auto split=j.find('\n');if(split==std::string::npos){e="Journal illisible : intervention nécessaire.";return false;}
    auto next=j.substr(0,split),old=j.substr(split+1);
    if(!versionOk(next)||!versionOk(old)){e="Versions du journal invalides.";return false;}
    auto prev=game()+".previous";
    if(exists(staged())){
        if(exists(prev)){if(exists(game())||rename(prev.c_str(),game().c_str())){e="Impossible de récupérer le jeu précédent.";return false;}}
        if(remove(staged().c_str())){e="Impossible de nettoyer le téléchargement.";return false;}
    }else{
        if(!valid3dsx(game())){e="Jeu installé absent ou invalide ; fichiers conservés.";return false;}
        if(!writeText(game()+".version",next)){e="Écriture de version impossible.";return false;}
        if(exists(prev)){
            if(exists(backup())&&remove(backup().c_str())){e="Ancienne copie de secours inaccessible.";return false;}
            std::string hash;uint64_t size=0;
            if(!hashFile(prev,hash,size)||!writeText(backup()+".sha256",hash)){e="Vérification du secours impossible.";return false;}
            if(!writeText(backup()+".version",old)||rename(prev.c_str(),backup().c_str())){e="Finalisation de la copie de secours impossible.";return false;}
        }
    }
    if(remove((game()+".journal").c_str())){e="Impossible de terminer le journal.";return false;}return true;
}
bool Storage::recover(std::string&e){if(busy())return finish(readText(game()+".journal"),e);if(exists(game()+".previous")){e="Copie de récupération sans journal : fichiers conservés.";return false;}return true;}
bool Storage::install(const std::string&v,std::string&e){
    if(!versionOk(v)||busy()||exists(game()+".previous")||!valid3dsx(staged())){e="Installation non prête ; fichiers conservés.";return false;}
    if(!writeText(game()+".journal",v+"\n"+version())){e="Impossible de préparer le journal.";return false;}
    if(exists(game())&&rename(game().c_str(),(game()+".previous").c_str())){e="Impossible de déplacer le jeu actuel.";return false;}
    if(rename(staged().c_str(),game().c_str())){e="Installation interrompue ; relancer le lanceur.";return false;}
    return finish(readText(game()+".journal"),e);
}
bool Storage::restore(std::string&e,const std::function<bool(uint64_t,uint64_t)>& progress){
    if(busy()||!valid3dsx(backup())){e="Aucune copie de secours utilisable.";return false;}
    FILE*in=fopen(backup().c_str(),"rb");if(!in){e="Lecture du secours impossible.";return false;}
    FILE*out=fopen(partial().c_str(),"wb");if(!out){fclose(in);e="Écriture SD impossible.";return false;}
    struct stat s{};stat(backup().c_str(),&s);uint64_t count=0;std::vector<char> b(32768);size_t n;bool ok=true;Sha256 hash;
    while((n=fread(b.data(),1,b.size(),in))){if(fwrite(b.data(),1,n,out)!=n){ok=false;break;}hash.add(b.data(),n);count+=n;if(!progress(count,s.st_size)){ok=false;break;}}
    ok=!ferror(in)&&ok;fclose(in);ok=closeWrite(out)&&ok;
    std::string original;uint64_t size=0;ok=ok&&hashFile(backup(),original,size)&&original==hash.finish()&&size==count;
    auto expected=readText(backup()+".sha256");
    if(expected.empty()||original!=expected){ok=false;e="Copie de secours altérée : restauration refusée.";}
    if(!ok){remove(partial().c_str());if(e.empty())e="Restauration annulée ou erreur de copie.";return false;}
    if(rename(partial().c_str(),staged().c_str())){e="Préparation de la restauration impossible.";return false;}
    return install(backupVersion(),e);
}
