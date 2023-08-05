#!/usr/bin/env python
# Modified version of Nathan Hamiel's REST echo server

from http.server import HTTPServer, BaseHTTPRequestHandler


class RequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        request_path = self.path

        print("\n----- Request Start ----->\n")
        print("Request path:", request_path)
        print("Request headers:", self.headers)
        print("<----- Request End -----\n")

        self.send_response(200)
        self.send_header("Set-Cookie", "foo=bar")
        self.end_headers()

    def do_POST(self):
        request_path = self.path

        print("\n----- Request Start ----->\n")
        print("Request path:", request_path)

        request_headers = self.headers
        content_length = request_headers.get("Content-Length")
        length = int(content_length) if content_length else 0
        payload = self.rfile.read(length)

        print("Content Length:", length)
        print("Request headers:", request_headers)
        print("Request payload:", payload)
        print("<----- Request End -----\n")

        self.send_response(200)
        self.end_headers()
        self.wfile.write(payload)

    do_PUT = do_POST
    do_DELETE = do_GET
    do_HEAD = do_GET


def main():
    port = 9000
    print("Listening on 0.0.0.0:%s" % port)
    server = HTTPServer(("", port), RequestHandler)

    for i in range(0, 4):
        server.handle_request()


if __name__ == "__main__":
    main()
