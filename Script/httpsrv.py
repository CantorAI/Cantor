from copyreg import constructor
import cantor
import asyncio
from http.server import HTTPServer,SimpleHTTPRequestHandler,BaseHTTPRequestHandler
import ssl
from socketserver import ThreadingMixIn
import threading
import logging,os
from urllib.parse import urlparse
from pathlib import Path

from requests import request

rootFolder = ""

class ThreadingSimpleServer(ThreadingMixIn, HTTPServer):
    pass

class MyServer(SimpleHTTPRequestHandler):
    def do_GET(self):
        print(self.path)
        SimpleHTTPRequestHandler.do_GET(self)

class Mime:
    img_mime = {
        "jpg": "image/jpeg",
        "jpeg": "image/jpeg",
        "png": "image/png",
        "gif": "image/gif"}
    html_mime ={
        "html":"text/html",
        "htm":"text/html",
        "css":"text/css",
        "js":"application/x-javascript",
    }
    def __init__(self,filename):
        self.isText = False
        self.contentType ="application/octet-stream" #default

        ext = filename[filename.rfind(".")+1:].lower()
        if ext in self.html_mime.keys():
            self.isText = True
            self.contentType = self.html_mime[ext]
        elif ext in self.img_mime.keys():
            self.isText = False
            self.contentType = self.img_mime[ext]
                    

    
class CantorServer(BaseHTTPRequestHandler):
    def do_GET(self):
        print(self.path)
        urlInfo = urlparse(self.path)
        requestFile = urlInfo.path
        query = urlInfo.query
        if query !='':
            query_components = dict(qc.split("=") for qc in query.split("&"))
        if requestFile.startswith("/"):
            requestFile = requestFile[1:]   
        mime = Mime(requestFile)         
        self.send_response(200)
        self.send_header("Content-type", mime.contentType)
        self.end_headers()
        fullPath = rootFolder.joinpath(requestFile)
        openMode = 'rb'
        if mime.isText:
            openMode ='rt'
        f = open(fullPath,openMode)
        content = f.read()
        f.close()
        if mime.isText:
            content = bytes(content.encode())
        self.wfile.write(content)

    def do_POST(self): #test by curl --insecure -d "foo=bar&bin=go" https://localhost
        content_length = int(self.headers['Content-Length']) # <--- Gets the size of data
        post_data = self.rfile.read(content_length) # <--- Gets the data itself
        logging.info("POST request,\nPath: %s\nHeaders:\n%s\n\nBody:\n%s\n",
                str(self.path), str(self.headers), post_data.decode('utf-8'))

        self.wfile.write("POST request for {}".format(self.path).encode('utf-8'))

def RunServer():
    global rootFolder
    rootFolder = Path(__file__).parent
    rootFolder = Path("C:/Dev/Cantor")
    port = 443
    httpd = ThreadingSimpleServer(('localhost', port), CantorServer)
    sslctx = ssl.SSLContext()
    sslctx.check_hostname = False # If set to True, only the hostname that matches the certificate will be accepted
    sslctx.load_cert_chain(certfile='C:\\Dev\\Cantor\\Script\\ssl\\cert.pem', keyfile="C:\\Dev\\Cantor\\Script\\ssl\\key.pem",
        password="shawn")
    httpd.socket = sslctx.wrap_socket(httpd.socket, server_side=True)
    httpd.serve_forever()


def StartServer():
    srv = threading.Thread(target=RunServer)
    srv.start()
