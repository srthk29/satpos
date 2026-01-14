add_subdirectory(generated)

find_package(OpenSSL REQUIRED)

add_executable(satpos
    main.cpp
    client.h
    client.cpp
    parse.cpp
    orbital_elements.cpp
    parse_tle.cpp
    server.h
    server.cpp
)

target_compile_definitions(satpos PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

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
    OpenSSL::SSL
    OpenSSL::Crypto
)
