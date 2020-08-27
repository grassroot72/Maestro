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
$ ab -k -n 5000 -c 100 http://localhost:9000/bench.html 
 
You may also want to put the compression option on,
$ ab -k -n 5000 -c 100 -H "Accept-Encoding: deflate" http://localhost:9000/bench.html 
 
Please replace localhost to your hostname if you would like to test on 
other computers.
 
The following is the test result on my very low-end laptop,
** CPU: Intel Celeron N2930 (4 cores) at ~2.16 GHz (2M Cache) **
** MEM: 4G **
** NET: laptop built-in wireless **

Server Software:        Maestro/1.0
Server Hostname:        warrior
Server Port:            9000

Document Path:          /bench.html
Document Length:        142 bytes

Concurrency Level:      100
Time taken for tests:   10.981 seconds
Complete requests:      5000
Failed requests:        0
Keep-Alive requests:    5000
Total transferred:      1790000 bytes
HTML transferred:       710000 bytes
Requests per second:    455.31 [#/sec] (mean)
Time per request:       219.629 [ms] (mean)
Time per request:       2.196 [ms] (mean, across all concurrent requests)
Transfer rate:          159.18 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    5 125.4      0    7145
Processing:    35  204 138.3    147     880
Waiting:        8   69  23.4     66     544
Total:         36  209 186.3    147    7263

Percentage of the requests served within a certain time (ms)
  50%    147
  66%    169
  75%    193
  80%    230
  90%    462
  95%    527
  98%    587
  99%    650
 100%   7263 (longest request)

If you test it on most recently manufactured computers, you will get much 
better benchmark results.

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
services (POST/PUT/DELETE...) through built-in functions.

todo list:
HTTP/1.1 POST/PUT/DELETE
build-in cache to provide better GET performance
industry level logging
HTTPS
```
