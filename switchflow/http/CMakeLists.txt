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
)

TARGET_LINK_LIBRARIES(http util)
