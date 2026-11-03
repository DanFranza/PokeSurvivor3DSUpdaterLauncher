#!/usr/bin/env python3
"""Build pinned curl and mbedTLS with the devkitPro 3DS socket adaptations."""
import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import urllib.request

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--directory', type=Path, required=True, help='Working directory outside the source tree')
args = parser.parse_args()
root = args.directory.resolve(); root.mkdir(parents=True, exist_ok=True)
project = Path(__file__).resolve().parents[1]
devkit = Path(os.environ.get('DEVKITPRO', '/opt/devkitpro'))
compiler = devkit/'devkitARM/bin'
prefix = root/'prefix'; (prefix/'lib').mkdir(parents=True, exist_ok=True)
sources = [
    ('https://curl.se/download/curl-8.23.0.tar.xz', 'curl-8.23.0.tar.xz', '8dadd7d2536a314f982ad05a651b713f426bf7d02cc2c76cffb9796f2218e6c5'),
    ('https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.7/mbedtls-3.6.7.tar.bz2', 'mbedtls-3.6.7.tar.bz2', 'a7e8bcbec0e6f761b4af24f25677626b35f762f68eef79c08677a363212d11f6'),
]
for url, name, digest in sources:
    archive = root/name
    if not archive.exists():
        archive.write_bytes(urllib.request.urlopen(url, timeout=90).read())
    if hashlib.sha256(archive.read_bytes()).hexdigest() != digest:
        raise SystemExit('Invalid source digest: '+name)
    with tarfile.open(archive) as package:
        package.extractall(root, filter='data')
mbed = root/'mbedtls-3.6.7'
config = mbed/'include/mbedtls/mbedtls_config.h'
text = config.read_text()
for key in ['MBEDTLS_ENTROPY_HARDWARE_ALT', 'MBEDTLS_NO_PLATFORM_ENTROPY', 'MBEDTLS_PLATFORM_MS_TIME_ALT']:
    text = text.replace('//#define '+key, '#define '+key)
for key in ['MBEDTLS_NET_C', 'MBEDTLS_TIMING_C']:
    text = text.replace('#define '+key, '//#define '+key)
config.write_text(text)
flags = '-O2 -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -ffunction-sections -fdata-sections -D__3DS__'
subprocess.run(['make', '-C', str(mbed/'library'), '-j4', 'CC='+str(compiler/'arm-none-eabi-gcc'),
                'AR='+str(compiler/'arm-none-eabi-ar'), 'CFLAGS='+flags], check=True)
for part in ['mbedtls', 'psa']:
    shutil.copytree(mbed/'include'/part, prefix/'include'/part, dirs_exist_ok=True)
for path in (mbed/'library').glob('*.a'):
    shutil.copy2(path, prefix/'lib'/path.name)
# Configure probes must link the same entropy/time adapter as the application.
subprocess.run([str(compiler/'arm-none-eabi-gcc'), '-c', str(project/'source/tls_platform.c'),
                '-o', str(root/'tls-platform.o'), *flags.split(), '-I'+str(devkit/'libctru/include'),
                '-I'+str(prefix/'include')], check=True)
subprocess.run([str(compiler/'arm-none-eabi-ar'), 'rcs', str(prefix/'lib/lib3dsentropy.a'), str(root/'tls-platform.o')], check=True)
curl = root/'curl-8.23.0'
# Port of the socket fixes from devkitPro's curl-8.4.0.patch:
# soc:U returns short sockaddr lengths and does not implement these socket options.
path = curl/'lib/curl_addrinfo.c'
text = path.read_text().replace('    if((size_t)ai->ai_addrlen < ss_size)\n      continue;',
        '#ifndef __3DS__\n    if((size_t)ai->ai_addrlen < ss_size)\n      continue;\n#endif')
path.write_text(text)
path = curl/'lib/cf-socket.c'; text = path.read_text().replace('#if defined(HAVE_FCNTL) && !defined(SOCK_CLOEXEC)', '#if defined(HAVE_FCNTL) && !defined(SOCK_CLOEXEC) && !defined(__3DS__)').replace('#ifdef SOCK_CLOEXEC\n  addr->socktype', '#if defined(SOCK_CLOEXEC) && !defined(__3DS__)\n  addr->socktype')
for name in ['tcpkeepalive', 'verifyconnect']:
    start = text.index('static '+('void' if name == 'tcpkeepalive' else 'bool')+' '+name+'(')
    begin = text.index('{', start); depth = 1; end = begin+1
    while depth:
        if text[end] == '{': depth += 1
        elif text[end] == '}': depth -= 1
        end += 1
    body = text[begin+1:end-1]
    stub = '\n  (void)cf; (void)data; (void)sockfd;\n' if name == 'tcpkeepalive' else '\n  (void)sockfd; *psockerr=0; return TRUE;\n'
    text = text[:begin+1]+'\n#ifdef __3DS__'+stub+'#else'+body+'#endif\n'+text[end-1:]
path.write_text(text)
env = dict(os.environ, PATH=str(compiler)+os.pathsep+os.environ['PATH'],
           CC='arm-none-eabi-gcc', AR='arm-none-eabi-ar', RANLIB='arm-none-eabi-ranlib', CFLAGS=flags,
           CPPFLAGS='-I'+str(devkit/'libctru/include'),
           LDFLAGS='-L'+str(devkit/'libctru/lib')+' -L'+str(prefix/'lib')+' -specs=3dsx.specs',
           LIBS='-l3dsentropy -lctru')
disabled = 'shared ipv6 unix-sockets threaded-resolver pthreads socketpair manual docs ldap ldaps ftp file dict telnet tftp pop3 imap smtp gopher rtsp smb mqtt websockets proxy netrc cookies alt-svc hsts'
without = 'zlib brotli zstd libpsl libidn2 nghttp2 libssh2 openssl'
subprocess.run(['./configure', '--host=arm-none-eabi', '--prefix='+str(prefix), '--enable-static',
                *('--disable-'+x for x in disabled.split()), *('--without-'+x for x in without.split()),
                '--with-mbedtls='+str(prefix)], cwd=curl, env=env, check=True)
subprocess.run(['make', '-C', 'lib', '-j4'], cwd=curl, env=env, check=True)
shutil.copy2(curl/'lib/.libs/libcurl.a', prefix/'lib/libcurl.a')
shutil.copytree(curl/'include/curl', prefix/'include/curl', dirs_exist_ok=True)
print('NETWORK_PREFIX='+str(prefix))
