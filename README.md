## ENV
linux amd64

## build
```
mkdir build && cd build
cmake .. && make -j8

# add test

cmake -DBUILD_TESTS=ON .. && make -j8
```