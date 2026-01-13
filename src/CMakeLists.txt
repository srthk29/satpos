add_subdirectory(generated)

add_executable(satpos
    client.cpp
    parse.cpp
    orbital_elements.cpp
    parse_tle.cpp
    server.h
    server.cpp
)

target_include_directories(satpos
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include/satpos
)

target_link_libraries(satpos
    PRIVATE
    simdjson
    sgp4
    httplib
    benchmark
    propsrv
)
