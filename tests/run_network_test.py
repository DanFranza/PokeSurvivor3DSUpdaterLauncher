"""Exercise the real curl implementation against a local, authenticated TLS server."""
from pathlib import Path
import http.server
import ssl
import subprocess
import threading
import tempfile
import sys
import time

root = Path(__file__).resolve().parents[1]
class Server(http.server.ThreadingHTTPServer):
    daemon_threads = True
class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, *_): pass
    def do_GET(self):
        try:
            if self.path in ('/redirect', '/relative', '/downgrade', '/loop'):
                target = {'/redirect': 'https://localhost/ok', '/relative': '/ok',
                          '/downgrade': 'http://localhost/ok', '/loop': '/loop'}[self.path]
                self.send_response(302); self.send_header('Location', target); self.end_headers()
            elif self.path == '/missing':
                self.send_response(404); self.end_headers()
            elif self.path == '/truncated':
                self.send_response(200); self.send_header('Content-Length', '1000'); self.end_headers(); self.wfile.write(b'short')
            elif self.path == '/slow':
                time.sleep(.8); self.send_response(200); self.end_headers(); self.wfile.write(b'slow')
            else:
                self.send_response(200); self.send_header('Content-Length', '65536'); self.end_headers()
                self.wfile.write(b'A' * 65536)
        except (OSError, ssl.SSLError): pass

with tempfile.TemporaryDirectory(dir=root/'build-tests') as directory:
    work = Path(directory).resolve()
    subprocess.run(['openssl', 'req', '-x509', '-newkey', 'rsa:2048', '-nodes',
                    '-keyout', str(work/'key.pem'), '-out', str(work/'cert.pem'),
                    '-days', '2', '-subj', '/CN=localhost',
                    '-addext', 'subjectAltName=DNS:localhost'], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    (work/'romfs:').mkdir(); (work/'romfs:/roots.pem').write_bytes((work/'cert.pem').read_bytes())
    server = Server(('127.0.0.1', 0), Handler)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(work/'cert.pem', work/'key.pem')
    server.socket = context.wrap_socket(server.socket, server_side=True)
    thread = threading.Thread(target=server.serve_forever, daemon=True); thread.start()
    try:
        subprocess.run([str(root/'build-tests/network-tests'), str(server.server_port)], cwd=work, check=True, timeout=30)
    finally:
        server.shutdown(); server.server_close()
