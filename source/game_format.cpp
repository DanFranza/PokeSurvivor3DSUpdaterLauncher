#include "game_format.h"
#include <cstdio>
#include <sys/stat.h>

const char* gameFormatCode(GameFormat format){return format==GameFormat::CIA?"cia":"3dsx";}
bool parseGameFormat(const std::string& text,GameFormat& format){auto code=text;while(!code.empty()&&(code.back()=='\n'||code.back()=='\r'))code.pop_back();if(code=="cia"){format=GameFormat::CIA;return true;}if(code=="3dsx"){format=GameFormat::ThreeDSX;return true;}return false;}
namespace {
uint32_t le32(const unsigned char* p){return uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16|uint32_t(p[3])<<24;}
uint64_t le64(const unsigned char* p){return le32(p)|uint64_t(le32(p+4))<<32;}
uint32_t be32(const unsigned char* p){return uint32_t(p[0])<<24|uint32_t(p[1])<<16|uint32_t(p[2])<<8|p[3];}
uint64_t be64(const unsigned char* p){return uint64_t(be32(p))<<32|be32(p+4);}
uint64_t align64(uint64_t n){return(n+63)&~uint64_t(63);}
}
bool validGameCia(const std::string& path){
    FILE* file=fopen(path.c_str(),"rb");if(!file)return false;
    struct stat st{};unsigned char header[32]{};bool ok=fstat(fileno(file),&st)==0&&fread(header,1,sizeof header,file)==sizeof header;
    uint64_t headerSize=le32(header),certSize=le32(header+8),ticketSize=le32(header+12),tmdSize=le32(header+16),contentSize=le64(header+24);
    uint64_t tmdAt=align64(headerSize)+align64(certSize)+align64(ticketSize),contentAt=tmdAt+align64(tmdSize);
    uint64_t fileSize=uint64_t(st.st_size);
    ok=ok&&fileSize<=512ull*1024*1024&&certSize<=fileSize&&ticketSize<=fileSize&&contentSize<=fileSize&&headerSize>=0x2020&&headerSize<=0x100000&&tmdSize>=0x200&&tmdSize<=0x100000&&contentSize>=0x200&&contentAt<=fileSize&&contentSize<=fileSize-contentAt;
    unsigned char signature[4]{};
    if(ok)ok=fseek(file,long(tmdAt),SEEK_SET)==0&&fread(signature,1,4,file)==4;
    uint32_t type=be32(signature),body=0;
    if(type==0x10000||type==0x10003)body=0x240;else if(type==0x10001||type==0x10004)body=0x140;else if(type==0x10002||type==0x10005)body=0x80;else ok=false;
    unsigned char id[8]{};
    if(ok)ok=body+0xa0<=tmdSize&&fseek(file,long(tmdAt+body+0x4c),SEEK_SET)==0&&fread(id,1,8,file)==8&&be64(id)==GAME_CIA_TITLE_ID;
    unsigned char ncch[32]{};
    if(ok)ok=fseek(file,long(contentAt+0x100),SEEK_SET)==0&&fread(ncch,1,sizeof ncch,file)==sizeof ncch&&ncch[0]=='N'&&ncch[1]=='C'&&ncch[2]=='C'&&ncch[3]=='H'&&le64(ncch+0x18)==GAME_CIA_TITLE_ID;
    fclose(file);return ok;
}
