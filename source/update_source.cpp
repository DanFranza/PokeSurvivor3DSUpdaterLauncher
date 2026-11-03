#include "update_source.h"
#include "sha1.h"
#include "picojson.h"
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <vector>

const char* defaultUpdateUrl(){return "https://api.github.com/repos/DanFranza/pokesurvivor3DS/contents/3ds/distribution/pokesurvivor3ds.3dsx?ref=port-3ds-danfranza";}
std::string resolveUpdateSource(const std::string&config){
    if(config.empty())return defaultUpdateUrl();
    const std::string prefix="manifest_url=";
    if(config.compare(0,prefix.size(),prefix)!=0)return {};
    auto url=config.substr(prefix.size());while(!url.empty()&&(url.back()=='\n'||url.back()=='\r'))url.pop_back();
    if(url=="https://github.com/DanFranza/PokeSurvivor3DSUpdaterLauncher/releases/latest/download/pokesurvivor-update.ini")return defaultUpdateUrl();
    return url;
}
bool parseGitHubFile(const std::string&json,Release&r,std::string&e){
    try{
        picojson::value doc;auto err=picojson::parse(doc,json);
        if(!err.empty()||!doc.is<picojson::object>()){e="Réponse GitHub illisible.";return false;}
        auto stringValue=[&](const char*key){auto&v=doc.get(key);return v.is<std::string>()?v.get<std::string>():std::string();};
        auto sha=stringValue("sha"),url=stringValue("download_url");
        if(stringValue("type")!="file"||stringValue("path")!="3ds/distribution/pokesurvivor3ds.3dsx"||sha.size()!=40||!std::all_of(sha.begin(),sha.end(),[](char c){return(c>='0'&&c<='9')||(c>='a'&&c<='f');})||!secureUrl(url)||url.compare(0,34,"https://raw.githubusercontent.com/")!=0||!doc.get("size").is<double>()){
            e="Informations du fichier GitHub invalides.";return false;
        }
        double size=doc.get("size").get<double>();
        if(!std::isfinite(size)||size<32||size>512.0*1024*1024||std::floor(size)!=size){e="Taille du jeu GitHub invalide.";return false;}
        r={"3ds-"+sha.substr(0,12),url,"",uint64_t(size),sha};return true;
    }catch(const std::exception&){e="Réponse GitHub invalide.";return false;}
}
bool verifyReleaseFile(const std::string&path,const Release&r){
    if(!valid3dsx(path))return false;
    if(r.gitSha1.empty()){std::string hash;uint64_t size=0;return hashFile(path,hash,size)&&size==r.size&&hash==r.sha256;}
    FILE*f=fopen(path.c_str(),"rb");if(!f)return false;
    Sha1 hash;std::string prefix="blob "+std::to_string(r.size);hash.add(prefix.c_str(),prefix.size()+1);
    std::vector<char> buffer(32768);uint64_t size=0;size_t n;
    while((n=fread(buffer.data(),1,buffer.size(),f))){hash.add(buffer.data(),n);size+=n;}
    bool ok=!ferror(f);fclose(f);return ok&&size==r.size&&hash.finish()==r.gitSha1;
}
