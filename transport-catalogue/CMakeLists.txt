cmake_minimum_required(VERSION 3.11)

project(project_15 CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS svg.proto map_renderer.proto transport_catalogue.proto transport_router.proto)

set(DOMAIN domain.h domain.cpp)
set(GEO geo.h geo.cpp)
set(GRAPH graph.h)
set(JSON json_builder.h json_builder.cpp json_reader.h json_reader.cpp json.h json.cpp)
set(MAP_RENDERER map_renderer.h map_renderer.cpp map_renderer.proto)
set(RANGES ranges.h)
set(REQUEST_HANDLER request_handler.h request_handler.cpp)
set(ROUTER router.h)
set(SERIALIZATION serialization.h serialization.cpp)
set(SVG svg.h svg.cpp svg.proto)
set(TRANSPORT_CATALOGUE transport_catalogue.h transport_catalogue.cpp transport_catalogue.proto)
set(TRANSPORT_ROUTER transport_router.h transport_router.cpp transport_router.proto)

add_executable(transport_catalogue main.cpp 
                                   ${PROTO_SRCS}
                                   ${PROTO_HDRS}
                                   ${DOMAIN}
                                   ${GEO}
                                   ${GRAPH}
                                   ${JSON}
                                   ${MAP_RENDERER}
                                   ${RANGES}
                                   ${REQUEST_HANDLER}
                                   ${ROUTER}
                                   ${SERIALIZATION}
                                   ${SVG}
                                   ${TRANSPORT_CATALOGUE}
                                   ${TRANSPORT_ROUTER})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)