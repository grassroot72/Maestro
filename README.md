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
-- HTTP/1.1 HEAD method (static file)
-- HTTP/1.1 keep-alive (long connection, disconnected after timeouts)
-- built-in cache to provide better GET performance
-- deflate compression
-- download resumption

```

## Cloning
```
git clone https://github.com/grassroot72/Maestro.git
```

## Build & Run
```
$ make
...
$ ./maestro
```

## Test
```
After you run the webserver, you should see the following

listening on port [9000]

then, you can open firefox or other browsers and key in the url
http://localhost:9000
or
http://localhost:9000/bench.html

If to test download resumption, key in the following url,
http://localhost:9000/download/Business-English-for-Success.pdf

The download resumption has been only partial implemented, I am going to
improve it later...

```

## Benchmark
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
Document Length:        1025 bytes

Concurrency Level:      100
Time taken for tests:   12.017 seconds
Complete requests:      5000
Failed requests:        0
Keep-Alive requests:    5000
Total transferred:      6465000 bytes
HTML transferred:       5125000 bytes
Requests per second:    421.76 [#/sec] (mean)
Time per request:       237.101 [ms] (mean)
Time per request:       2.371 [ms] (mean, across all concurrent requests)
Transfer rate:          532.56 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    3  59.6      0    3229
Processing:    34  228 154.9    155     863
Waiting:       11   80  46.8     70     387
Total:         40  231 166.0    155    3365

Percentage of the requests served within a certain time (ms)
  50%    155
  66%    179
  75%    202
  80%    395
  90%    510
  95%    568
  98%    631
  99%    667
 100%   3365 (longest request)

If you test it on most recently manufactured computers, you will get much
better benchmark results.
```

## Note
```
I am not the author of the pdf file under the download folder, I only
use it for download resumption test.
```

## Todo
```
I want to develop an HTTP/1.1 compatible webserver as well as an application
server which skips traditional CGI function and directly provides dynamic
services (POST/PUT/DELETE...) through built-in functions.

todo list:
-- HTTP/1.1 POST/PUT/DELETE
-- improve download resumption
-- industry level logging
-- HTTPS
```
