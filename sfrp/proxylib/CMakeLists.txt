ADD_LIBRARY(proxylib 
proxy_handler.cpp
new_connection_handler.cpp
proxy_stream_interface.cpp
pipeline_data_queue.cpp
)

TARGET_LINK_LIBRARIES(proxylib util socketlib)
