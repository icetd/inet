## ENV
linux amd64

## build
```
mkdir build && cd build
cmake .. && make -j8

# add test

cmake -DBUILD_TESTS=ON .. && make -j8
```

## Static WebServer
Put your personal static website in the www directory