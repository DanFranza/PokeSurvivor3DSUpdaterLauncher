#pragma once
#include <cstdint>
using u8=uint8_t;using u32=uint32_t;using u64=uint64_t;using Result=int32_t;
#define R_SUCCEEDED(r) ((r)>=0)
#define R_FAILED(r) ((r)<0)
#define HTTPC_RESULTCODE_DOWNLOADPENDING 0xd840a02bu
#define HTTPC_RESULTCODE_TIMEDOUT 0xd820a069u
#define HTTPC_METHOD_GET 1
struct httpcContext{u32 httphandle;};
Result httpcInit(u32);void httpcExit();
Result httpcOpenContext(httpcContext*,int,const char*,u32);
Result httpcCloseContext(httpcContext*);
Result httpcAddTrustedRootCA(httpcContext*,const u8*,u32);
Result httpcAddRequestHeaderField(httpcContext*,const char*,const char*);
Result httpcBeginRequest(httpcContext*);
Result httpcGetResponseStatusCodeTimeout(httpcContext*,u32*,u64);
Result httpcGetResponseHeader(httpcContext*,const char*,char*,u32);
Result httpcGetDownloadSizeState(httpcContext*,u32*,u32*);
Result httpcReceiveDataTimeout(httpcContext*,u8*,u32,u64);
