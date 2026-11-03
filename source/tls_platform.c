#ifdef __3DS__
#include <3ds.h>
#include <mbedtls/entropy.h>
#include <mbedtls/platform_time.h>
#include <sys/time.h>

// Use the console's cryptographic random generator, and propagate any failure.
int mbedtls_hardware_poll(void* data,unsigned char* output,size_t len,size_t* olen){
    (void)data;
    *olen=0;
    Result result=sslcGenerateRandomData(output,len);
    if(R_FAILED(result))return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    *olen=len;return 0;
}
mbedtls_ms_time_t mbedtls_ms_time(void){
    return (mbedtls_ms_time_t)(svcGetSystemTick()/SYSCLOCK_ARM11*1000
        +(svcGetSystemTick()%SYSCLOCK_ARM11)*1000/SYSCLOCK_ARM11);
}
#endif
