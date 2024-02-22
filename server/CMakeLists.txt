add_executable(server src/server.cpp)
target_compile_features(server PRIVATE cxx_std_17)
target_link_libraries(server PRIVATE fcgi)
