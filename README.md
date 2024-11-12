## test ENV
linux amd64 && arrch64

## build
```
mkdir build && cd build
cmake .. && make -j8

# add test

cmake -DBUILD_TESTS=ON .. && make -j8
```

## Static WebServer
### config file

```
[server]
port = 8080
threadnum = 4
rootpath  = ./www

[log]
level = 3
; TRACE = 0,
; DEBUG = 1,
; INFO = 2,
; WARN = 3,
; ERROR = 4,
; FATAL = 5,
basename = server
```
