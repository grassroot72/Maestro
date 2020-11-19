# Multithread And Epoll Server To Revolve Online (Maestro)

This project aims to create a tiny but functional application server by
leveraging multithread and epoll features of linux OS.

All the source code is written in C.

The project is a WIP (Work In Progress) project. It is still in its infant
stage. More features will be added in the future.


## Features

  - Multithread pool
  - Epoll (so, linux specific)
  - HTTP/1.1 GET method (static file)
  - HTTP/1.1 HEAD method (static file)
  - HTTP/1.1 POST method
  - HTTP/1.1 chunked transfer
  - HTTP/1.1 keep-alive (long connection, disconnected after timeouts)
  - built-in cache to provide better GET performance
  - deflate compression
  - download resumption


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

After you run the webserver, you should see the following

**listening on port [9000]**

then, you can open firefox or other browsers and key in the url
http://localhost:9000
or
http://localhost:9000/bench.html

To test download resumption, key in the following url in your browser(firefox),
http://localhost:9000/download/Business-English-for-Success.pdf
refresh the browser if download stalls. Keep refresh several times until the
download finally finishes.


## Benchmark
```
For Apache Bench, I used the following command,
$ ab -k -n 500000 -c 1000 http://localhost:9000/bench.html

You may also want to put the compression option on,
$ ab -k -n 500000 -c 1000 -H "Accept-Encoding: deflate" http://localhost:9000/bench.html

Please replace localhost with your hostname if you would like to test on
other computers.
```

The following is the test result on my very low-end laptop,
  - **CPU: Intel Celeron N2930 (4 cores) at ~2.16 GHz (2M Cache)**
  - **MEM: 4G**
  - **NET: laptop built-in wireless**
```
Server Software:        Maestro/1.0
Server Hostname:        localhost
Server Port:            9000

Document Path:          /bench.html
Document Length:        1025 bytes

Concurrency Level:      1000
Time taken for tests:   34.536 seconds
Complete requests:      500000
Failed requests:        0
Keep-Alive requests:    500000
Total transferred:      616500000 bytes
HTML transferred:       460000000 bytes
Requests per second:    14477.83 [#/sec] (mean)
Time per request:       69.071 [ms] (mean)
Time per request:       0.069 [ms] (mean, across all concurrent requests)
Transfer rate:          17432.77 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   3.8      0     112
Processing:    26   69  18.5     65     169
Waiting:        4   43  13.5     45     146
Total:         26   69  18.8     65     169

Percentage of the requests served within a certain time (ms)
  50%     65
  66%     73
  75%     80
  80%     84
  90%     96
  95%    105
  98%    114
  99%    121
 100%    169 (longest request)

If you test it on most recently manufactured computers, you will get much
better benchmark results.
```


## Note

I am not the author of the pdf file under the download folder, I only
use it for download resumption test.


## Todo

I want to develop an HTTP/1.1 compatible webserver as well as an application
server which skips traditional CGI function and directly provides dynamic
services (POST) through built-in functions.

todo list:
  - HTTPS
