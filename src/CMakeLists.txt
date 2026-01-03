add_executable(satpos
    main.cpp
)

target_link_libraries(satpos
    PRIVATE
    simdjson
)

target_link_libraries(satpos
    PRIVATE
    sgp4
)

target_link_libraries(satpos
    PRIVATE
    cpp-httplib
)

target_link_libraries(satpos
    PRIVATE
    benchmark
)
