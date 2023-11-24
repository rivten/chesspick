add_executable(server src/server.cpp src/fastcgi.cpp)
target_compile_features(server PRIVATE cxx_std_17)
