# Create the library
add_library(webserver STATIC webserver.cpp)

# Tell CMake where to find headers
# "PUBLIC" means: visible to this library AND anything that links against it
target_include_directories(webserver
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

add_executable(mini_web_server main.cpp)
target_link_libraries(mini_web_server PRIVATE webserver)
