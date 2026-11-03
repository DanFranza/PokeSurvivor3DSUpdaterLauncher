#include "network.h"
#include <3ds.h>
#include <filesystem>
#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <cstdlib>
struct Response{u32 status;std::string body,location;bool timeout=false;};
static std::vector<Response> responses;
static size_t requestIndex=0,position=0;
static unsigned checks=0,opened=0,closed=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"Échec réseau ligne "<<__LINE__<<": "<<#x<<"\n";std::abort();}}while(0)
Result httpcInit(u32){return 0;}void httpcExit(){}
Result httpcOpenContext(httpcContext*c,int,const char*url,u32){CHECK(secureUrl(url));if(requestIndex>=responses.size())return -1;c->httphandle=requestIndex++;position=0;++opened;return 0;}
Result httpcCloseContext(httpcContext*){++closed;return 0;}
Result httpcAddTrustedRootCA(httpcContext*,const u8*,u32){return 0;}
Result httpcAddRequestHeaderField(httpcContext*,const char*,const char*){return 0;}
Result httpcBeginRequest(httpcContext*){return 0;}
Result httpcGetResponseStatusCodeTimeout(httpcContext*c,u32*s,u64){*s=responses[c->httphandle].status;return 0;}
Result httpcGetResponseHeader(httpcContext*c,const char*,char*out,u32 n){auto s=responses[c->httphandle].location;if(s.size()>=n)return -1;strcpy(out,s.c_str());return 0;}
Result httpcGetDownloadSizeState(httpcContext*c,u32*downloaded,u32*total){*downloaded=position;*total=responses[c->httphandle].body.size();return 0;}
Result httpcReceiveDataTimeout(httpcContext*c,u8*out,u32 n,u64){auto&r=responses[c->httphandle];if(r.timeout)return Result(HTTPC_RESULTCODE_TIMEDOUT);size_t count=std::min(size_t(n),r.body.size()-position);memcpy(out,r.body.data()+position,count);position+=count;return position<r.body.size()?Result(HTTPC_RESULTCODE_DOWNLOADPENDING):0;}
static bool download(const std::vector<Response>&r,uint64_t limit,uint64_t exact,bool cancel=false){responses=r;requestIndex=0;std::string error;bool ok=fetch("https://github.com/file","received.bin",limit,exact,error,[&](uint64_t,uint64_t){return !cancel;});CHECK(opened==closed);CHECK(ok||!exists("received.bin"));return ok;}
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
    std::cout<<checks<<" vérifications réseau réussies.\n";
}
