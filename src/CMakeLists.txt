add_executable(satpos
    client.cpp
)

target_include_directories(satpos
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(satpos
    PRIVATE
    simdjson
    sgp4
    httplib
    benchmark
)
