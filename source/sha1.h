#pragma once
#include <cstdint>
#include <cstring>
#include <string>
// SHA-1 is used solely to match GitHub's Git blob object ID (not signatures).
class Sha1 {
    uint32_t h[5]={0x67452301,0xefcdab89,0x98badcfe,0x10325476,0xc3d2e1f0};
    uint8_t block[64]{};size_t used=0;uint64_t count=0;
    static uint32_t rot(uint32_t x,int n){return (x<<n)|(x>>(32-n));}
    void compress(){
        uint32_t w[80];for(int i=0;i<16;i++)w[i]=uint32_t(block[i*4])<<24|uint32_t(block[i*4+1])<<16|uint32_t(block[i*4+2])<<8|block[i*4+3];
        for(int i=16;i<80;i++)w[i]=rot(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
        uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4];
        for(int i=0;i<80;i++){
            uint32_t f,k;if(i<20){f=(b&c)|(~b&d);k=0x5a827999;}else if(i<40){f=b^c^d;k=0x6ed9eba1;}else if(i<60){f=(b&c)|(b&d)|(c&d);k=0x8f1bbcdc;}else{f=b^c^d;k=0xca62c1d6;}
            uint32_t t=rot(a,5)+f+e+k+w[i];e=d;d=c;c=rot(b,30);b=a;a=t;
        }
        h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;
    }
public:
    void add(const void*p,size_t n){auto b=static_cast<const uint8_t*>(p);count+=n;while(n){size_t m=n<64-used?n:64-used;memcpy(block+used,b,m);used+=m;b+=m;n-=m;if(used==64){compress();used=0;}}}
    std::string finish(){uint64_t bits=count*8;uint8_t one=128,zero=0;add(&one,1);while(used!=56)add(&zero,1);uint8_t tail[8];for(int i=0;i<8;i++)tail[7-i]=bits>>(8*i);add(tail,8);const char*hex="0123456789abcdef";std::string out;for(auto x:h)for(int i=7;i>=0;i--)out+=hex[(x>>(i*4))&15];return out;}
};
