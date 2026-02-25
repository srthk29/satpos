# LEO's Satellite Position

## Two Line Elements

## SGP4

_Add the following line in sgp4's root CMakeLists.txt_
```cmake
target_include_directories(sgp4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libsgp4)
```

_Export CXX_INSTALL_DIR env_
```bash
export CXX_INSTALL_DIR=$HOME/.local
export PATH="$CXX_INSTALL_DIR/bin:$PATH"
```

_Build_
```bash
cmake -S . -B build
cmake --build "build" -j2
```

_Run_
```bash
./build/src/satpos
```
