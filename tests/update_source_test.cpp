#include "update_source.h"
#include "sha1.h"
#include <iostream>
#include <cstdlib>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"Échec source ligne "<<__LINE__<<": "<<#x<<"\n";std::abort();}}while(0)
int main(int argc,char**argv){
    Sha1 empty;CHECK(empty.finish()=="da39a3ee5e6b4b0d3255bfef95601890afd80709");
    Sha1 abc;abc.add("a",1);abc.add("bc",2);CHECK(abc.finish()=="a9993e364706816aba3e25717850c26c9cd0d89d");
    Sha1 big;std::string block(1000,'a');for(int i=0;i<1000;i++)big.add(block.data(),block.size());CHECK(big.finish()=="34aa973cd4c4daa4f61eeb2bdbad27316534016f");
    CHECK(resolveUpdateSource("")==defaultUpdateUrl());
    CHECK(resolveUpdateSource("manifest_url=https://github.com/DanFranza/PokeSurvivor3DSUpdaterLauncher/releases/latest/download/pokesurvivor-update.ini\r\n")==defaultUpdateUrl());
    CHECK(resolveUpdateSource("manifest_url=https://example.com/custom.ini\n")=="https://example.com/custom.ini");
    CHECK(resolveUpdateSource("bad config").empty());
    Release r;std::string error;auto json=readText("tests/fixtures/github-file.json");
    CHECK(parseGitHubFile(json,r,error));CHECK(r.size==48334904);CHECK(r.gitSha1=="3845b9ab4bb93ddd923555cd3329617addb96664");CHECK(r.version=="3ds-3845b9ab4bb9");CHECK(r.sha256.empty());
    for(auto bad:{"{}","[]","null","{bad json","{\"size\":1e999}"})CHECK(!parseGitHubFile(bad,r,error));
    auto malformed=json;malformed.replace(malformed.find("48334904"),8,"-1");CHECK(!parseGitHubFile(malformed,r,error));
    malformed=json;malformed.replace(malformed.find("https://raw.githubusercontent.com/"),34,"http://raw.githubusercontent.com/");CHECK(!parseGitHubFile(malformed,r,error));
    malformed=json;malformed.replace(malformed.find("\"type\": \"file\""),14,"\"type\": \"dir\"");CHECK(!parseGitHubFile(malformed,r,error));
    if(argc==2){CHECK(parseGitHubFile(json,r,error));CHECK(verifyReleaseFile(argv[1],r));auto wrong=r;wrong.gitSha1[0]='0';CHECK(!verifyReleaseFile(argv[1],wrong));wrong=r;wrong.size++;CHECK(!verifyReleaseFile(argv[1],wrong));std::cout<<"Binaire GitHub réel : taille, en-tête et empreinte Git validés.\n";}
    std::cout<<checks<<" vérifications de source réussies.\n";
}
