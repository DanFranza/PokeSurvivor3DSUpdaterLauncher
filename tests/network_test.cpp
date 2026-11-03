#include "network.h"
#include <3ds.h>
#include <filesystem>
#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <cstdlib>
struct Response{u32 status;std::string body,location;bool timeout=false;int headerWaits=0,readWaits=0;bool partial=false;};
static std::vector<Response> responses;
static size_t requestIndex=0,position=0;
static unsigned checks=0,opened=0,closed=0;
static u64 ticks=0;static std::vector<u64> waits;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"Échec réseau ligne "<<__LINE__<<": "<<#x<<"\n";std::abort();}}while(0)
Result httpcInit(u32){return 0;}void httpcExit(){}
Result httpcOpenContext(httpcContext*c,int,const char*url,u32){CHECK(secureUrl(url));if(requestIndex>=responses.size())return -1;c->httphandle=requestIndex++;position=0;++opened;return 0;}
Result httpcCloseContext(httpcContext*){++closed;return 0;}
Result httpcAddTrustedRootCA(httpcContext*,const u8*,u32){return 0;}
Result httpcAddRequestHeaderField(httpcContext*,const char*,const char*){return 0;}
Result httpcBeginRequest(httpcContext*){return 0;}
Result httpcGetResponseStatusCodeTimeout(httpcContext*c,u32*s,u64 timeout){waits.push_back(timeout);auto&r=responses[c->httphandle];if(r.headerWaits>0){--r.headerWaits;ticks+=timeout;return Result(HTTPC_RESULTCODE_TIMEDOUT);}*s=r.status;return 0;}
Result httpcGetResponseHeader(httpcContext*c,const char*,char*out,u32 n){auto s=responses[c->httphandle].location;if(s.size()>=n)return -1;strcpy(out,s.c_str());return 0;}
Result httpcGetDownloadSizeState(httpcContext*c,u32*downloaded,u32*total){*downloaded=position;*total=responses[c->httphandle].body.size();return 0;}
Result httpcReceiveDataTimeout(httpcContext*c,u8*out,u32 n,u64 timeout){waits.push_back(timeout);auto&r=responses[c->httphandle];if(r.timeout||r.readWaits>0){if(r.readWaits>0)--r.readWaits;ticks+=timeout;if(r.partial){size_t part=std::min(size_t(3),r.body.size()-position);memcpy(out,r.body.data()+position,part);position+=part;}return Result(HTTPC_RESULTCODE_TIMEDOUT);}size_t count=std::min(size_t(n),r.body.size()-position);memcpy(out,r.body.data()+position,count);position+=count;return position<r.body.size()?Result(HTTPC_RESULTCODE_DOWNLOADPENDING):0;}
static bool download(const std::vector<Response>&r,uint64_t limit,uint64_t exact,bool cancel=false,const FetchOptions& options={}){responses=r;requestIndex=0;std::string error;bool ok=fetch("https://github.com/file","received.bin",limit,exact,error,[&](uint64_t,uint64_t){return !cancel;},options);CHECK(opened==closed);CHECK(ok||!exists("received.bin"));return ok;}
int main(){
    std::filesystem::create_directories("romfs:");
    std::ofstream roots("romfs:/roots.bin",std::ios::binary);const char dummy[]={1,0,0,0,42};roots.write(dummy,sizeof dummy);roots.close();
    std::string body(100000,'A');body.back()='Z';
    CHECK(download({{200,body,""}},body.size(),body.size()));CHECK(readText("received.bin").empty()); // readText is intentionally limited to manifest sizes.
    std::ifstream file("received.bin",std::ios::binary);std::string data((std::istreambuf_iterator<char>(file)),{});CHECK(data==body);
    CHECK(download({{302,"","https://release-assets.githubusercontent.com/file"},{200,body,""}},body.size(),body.size()));
    CHECK(download({{307,"","/redirect"},{200,"abc",""}},3,3));
    CHECK(!download({{302,"","http://evil.example/file"}},100,0));
    CHECK(!download(std::vector<Response>(9,{302,"","https://github.com/loop"}),100,0));
    CHECK(!download({{404,"not found",""}},100,0));
    CHECK(!download({{200,body,""}},100,0));
    CHECK(!download({{200,"short",""}},100,100));
    CHECK(!download({{200,body,""}},body.size(),body.size(),true));
    CHECK(!download({{200,"","",true}},100,0));
    waits.clear();FetchOptions options;options.pollNs=100000000;options.budgetNs=500000000;options.nowNs=[](){return ticks;};ticks=0;
    CHECK(download({{200,"abc","",false,2}},3,3,false,options));CHECK(ticks==200000000);
    ticks=0;CHECK(download({{200,"abcdefghij","",false,0,2,true}},10,10,false,options));CHECK(readText("received.bin")=="abcdefghij");
    ticks=0;options.budgetNs=300000000;CHECK(!download({{200,"","",true}},100,0,false,options));CHECK(ticks==300000000);
    ticks=0;options.budgetNs=250000000;CHECK(!download({{200,"","",false,100}},100,0,false,options));CHECK(ticks==250000000);
    ticks=0;options.budgetNs=1000000000;options.cancelled=[](){return ticks>=100000000;};CHECK(!download({{200,"","",false,100}},100,0,false,options));CHECK(ticks==100000000);
    options.cancelled={};options.budgetNs=350000000;ticks=0;CHECK(!download({{302,"","https://github.com/next",false,2},{200,"abc","",false,2}},3,3,false,options));CHECK(ticks==350000000);
    for(auto wait:waits)CHECK(wait<=100000000);
    std::cout<<checks<<" vérifications réseau réussies.\n";
}
