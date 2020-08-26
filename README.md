# Multithread And Epoll Server To Revolve Online (Maestro)
```
This project aims to create a tiny but functional application server by 
leveraging multithread and epoll features of linux OS. 
 
All the source code is written in C. 
 
The project is a WIP (Work In Progress) project. It is still in its infant 
stage. More features will be added in the future.

```

## Features
```
-- Multithread pool
-- Epoll (so, linux specific)
-- HTTP/1.1 GET method (static file)
-- HTTP/1.1 keep-alive (long connection, disconnected after timeouts)
-- deflate compression
-- download resumption

```

## Cloning
```
git clone https://github.com/grassroot72/Maestro.git
```

## Building
```
$ make
...
$ ./maestro
```

## Testing
```
Running Apache Bench, I used the following command,
$ ab -k -n 50000 -c 800 http://localhost:9000/bench.html 
 
You may also want to put the compression option on,
$ ab -k -n 50000 -c 800 -H "Accept-Encoding: deflate" http://localhost:9000/bench.html 
 
Please replace localhost to your hostname if you would like to test on 
other computers.
 
The following is the test result on my very low-end laptop,
to be updated ...
...

To test download resumption, uncomment #define DEBUG and build again, then,
$ ./maestro 
resumption still needs a lot of optimization, but it works with firefox.
```

## Note
```
I am not the author of the pdf file under the download folder, I only 
use it for download resumption test.
```

## Todo
```
I want to develop an HTTP/1.1 compatible webserver as well as an application 
server which will skip traditional CGI function and directly provide dynamic 
services (POST/PUT/DELETE...) through build-in functions.

todo list:
HTTP/1.1 POST/PUT/DELETE
build-in cache to provide better GET performance
industry level logging
HTTPS
```
