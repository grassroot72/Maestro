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
Time taken for tests:   12.456 seconds
Complete requests:      5000
Failed requests:        0
Keep-Alive requests:    5000
Total transferred:      1790000 bytes
HTML transferred:       710000 bytes
Requests per second:    401.41 [#/sec] (mean)
Time per request:       249.120 [ms] (mean)
Time per request:       2.491 [ms] (mean, across all concurrent requests)
Transfer rate:          140.34 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    6 160.5      0    7327
Processing:    30  232 157.0    172    1013
Waiting:        8   83  43.7     67     399
Total:         32  238 223.3    172    7428

Percentage of the requests served within a certain time (ms)
  50%    172
  66%    216
  75%    261
  80%    303
  90%    507
  95%    596
  98%    691
  99%    753
 100%   7428 (longest request)

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
