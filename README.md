# HTTP/1.1 server using Linux Apis

## Features -

1. Concurrent connetions.

   1. Multithreading + thread pooling
      Instead of creating a new thread for every new connection, we create a thread pool with MAX_THREAD(in constans.h) # of thread when the server starts.
      This is beneficial as - creating new thread takes considerable time - A limit on the # of threads is set
      A queue is used to used to store new connections.
      All the threads dont go on loop, looking for work in the queue, but wait until new connection is added to queue/already present connection needs to be handled. This is done using condition variable.

   2. Condition Varibles to invoke thread only when work comes.

2. HTTP/1.1 specific-
   1. Methods -
      1. GET
      2. HEAD
      3. For other types of request, returns '501 Not Implemented' status
   2. Headers -
      1. Server - {linuxServer}
      2. Content-type - {text/html, application/jpeg, apllication/png}
      3. Date - {in GMT}
      4. If-Modified-Since
      5. Connection - {keep-alive,close}
      6. Content-Length
   3. Files supported -
      1. HTML
      2. jpg/jpeg/png
   4. Persistent connetions
      1. If the Connection header of the request is 'keep-alive', then the connection is not closed after sending the response. If the server gets no request within TIMEOUT(in constants.h), the connection is closed.
      2. If the header- Connection is 'close'. Then the connection is closed immediately after serving the request.
   5. If-Modified-Since
      1. If the If-Modified-Since header value is equal to the last modified time of the requested resource, then a '304 Not Modified' reponse is send.
      2. Otherwise the resource is served to client.
   6. Not Implemented reponse
      1. If the HTTP method requested by the client is not implemented by the server, a '501 Not Implemented' response is sent.
   7. Not found
      1. If the file does not exist then a '404 Not Found' response is sent.

Function description -
thread_fun: work assigned to eah thread.
handle connection: handle the client connection. Reads the request and processes and sends the response.
httpserver class: the HTTP/1.1 compliant server with features as described above.
queue header file: contains the queue data structure
constants header file: contains the SERVER constanst.


## Running -
```bash

g++ -pthread -o server server.cpp
./server

To load html page -
go to - http://localhost:8989/index.html

To get an image -
go to - http://localhost:8989/image.jpg
```
## Results -
specification:

- 100,000 threads : can be changed from constants.h
- 1,000,000 requests
- 20,000 concurrent requests :this was the maximum limit of the tool which i was using, although it can be increased till MAX_THREADS

---

##### An empty HTML file is sent via HTTP/1.1(i.e only status line and headers) for benchmarking.

---
<img src="https://github.com/Krutikmehta/Http1.1-server/blob/master/http-server/100k_threads.png" height=900  width=900/>

### Results can be found in http_server/100K_threads.png.
