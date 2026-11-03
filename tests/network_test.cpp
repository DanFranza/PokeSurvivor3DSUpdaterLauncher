#include "network.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"Network check failed at "<<__LINE__<<": "<<#x<<"\n";std::abort();}}while(0)
int main(int argc,char**argv){
    CHECK(argc==2);
    setenv("LAUNCHER_TEST_ROUTE",(std::string(":443:127.0.0.1:")+argv[1]).c_str(),1);
    auto get=[&](const std::string& path,uint64_t limit,uint64_t exact,const FetchOptions& options=FetchOptions{}){
        std::string error;
        bool ok=fetch("https://localhost/"+path,"received.bin",limit,exact,error,[](uint64_t,uint64_t){return true;},options);
        if(!ok)std::cerr<<path<<": "<<error<<"\n";CHECK(ok||!exists("received.bin"));return ok;
    };
    CHECK(get("ok",100000,65536));
    CHECK(std::filesystem::file_size("received.bin")==65536);
    std::ifstream file("received.bin",std::ios::binary);std::string data((std::istreambuf_iterator<char>(file)),{});
    CHECK(data==std::string(65536,'A'));
    CHECK(get("redirect",100000,65536));
    CHECK(get("relative",100000,65536));
    CHECK(!get("downgrade",100000,0));
    CHECK(!get("loop",100000,0));
    CHECK(!get("missing",100000,0));
    CHECK(!get("ok",100,0));
    CHECK(!get("ok",100000,65537));
    CHECK(!get("truncated",100000,1000));
    std::string error;
    CHECK(!fetch("http://localhost/ok","received.bin",100000,0,error,[](uint64_t,uint64_t){return true;}));
    CHECK(!fetch("https://wrong.test/ok","received.bin",100000,0,error,[](uint64_t,uint64_t){return true;}));
    CHECK(error.find("date")!=std::string::npos);
    FetchOptions budget;budget.budgetNs=150000000;budget.pollNs=20000000;
    CHECK(!get("slow",100000,0,budget));
    unsigned polls=0;FetchOptions cancel;cancel.cancelled=[&]{return ++polls>=5;};cancel.pollNs=20000000;
    CHECK(!get("slow",100000,0,cancel));
    CHECK(polls>=5);
    CHECK(!fetch("https://localhost/ok","received.bin",100000,0,error,[](uint64_t,uint64_t){return false;}));
    CHECK(!exists("received.bin"));
    std::filesystem::rename("romfs:/roots.pem","romfs:/roots.saved");
    CHECK(!get("ok",100000,0));
    std::filesystem::rename("romfs:/roots.saved","romfs:/roots.pem");
    std::cout<<checks<<" real HTTPS network checks passed.\n";
}
