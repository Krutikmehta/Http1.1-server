HTTP/1.1 server

Features - 
1. Concurrent connetions.
    1. Multithreading + thread pooling
    2. Condition Varibles to invoke thread only when work comes.
2. HTTP/1.1 specific-
    1. Methods -
        1. GET
        2. HEAD
        3. For other types of request, returns '501 Not Implemented' status
    2. Headers - 
        1. Server
        2. Content-type - {text/html, application/jpeg, apllication/png}
        3. Date
        4. If-Modified-Since
        5. Connection - {keep-alive,close}
        6. Content-Length
    3. Files supported -
        1. HTML
        2. jpg/jpeg/png
    4. Persistent connetions 
        1. If the Connection header of the request is 'keep-alive', then the connection is not closed after sending the response. If no the server gets no request within TIMEOUT=2s, the connection is closed.
        2. If the header is 'close'. Then the connection is closed immediately after serving the request.
    5. If-Modified-Since
        1. If the If-Modified-Since header value is equal to the last modified time of the requested resource, then a '304 Not Modified' reponse is send.
        2. Otherwise the resource is served to client.
    6. Not Implemented reponse
        1. If the HTTP method is not implemented by the server, a '501 Not Implemented' response is sent.
    7. Not found
        1. If the file does not exist then a '404 Not Found' response is sent.