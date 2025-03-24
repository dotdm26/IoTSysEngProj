from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib.parse

# Store the latest user status received
latest_status = "No status received yet."

class RequestHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        global latest_status
        
        # Parse URL
        parsed_path = urllib.parse.urlparse(self.path)
        query_params = urllib.parse.parse_qs(parsed_path.query)
        
        if parsed_path.path == '/submit':
            # Handle status submission
            latest_status = query_params.get('status', ['UNKNOWN'])[0]
            print(f"Received user_status: {latest_status}")
            
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            self.wfile.write(b"Status received successfully!")
        
        elif parsed_path.path == '/status':
            # Serve the latest status as plain text
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            self.wfile.write(latest_status.encode())
        
        elif parsed_path.path == '/':
            # Serve the HTML page
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            with open('index.html', 'rb') as file:
                self.wfile.write(file.read())
        
        else:
            self.send_response(404)
            self.end_headers()

if __name__ == "__main__":
    PORT = 8000
    server_address = ('', PORT)
    httpd = HTTPServer(server_address, RequestHandler)
    print(f"Server running on port {PORT}")
    httpd.serve_forever()
