ADD_LIBRARY(http
http.cpp 
header_parser.cpp 
body_parser.cpp
chunked_body_parser.cpp
content_length_body_parser.cpp
end_connection_body_parser.cpp
url.cpp	
header_cache.cpp
header_handler.cpp
header_writer.cpp
message_buffer.cpp
header_buffer.cpp
request_buffer_wrapper.cpp
response_buffer_wrapper.cpp
error_response.cpp
static_strings.cpp
start_line_parser.cpp
uri_query_parser.cpp
)

TARGET_LINK_LIBRARIES(http util)

ADD_EXECUTABLE(start_line_test tests/start_line_test.cpp)
TARGET_LINK_LIBRARIES(start_line_test http util pthread boost_system)

ADD_EXECUTABLE(uri_query_test tests/uri_query_test.cpp)
TARGET_LINK_LIBRARIES(uri_query_test http util pthread boost_system)
