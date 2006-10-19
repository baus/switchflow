ADD_LIBRARY(proxylib 
ProxyHandler.cpp
NewConnectionHandler.cpp
ProxyStreamInterface.cpp
pipeline_data_queue.cpp
)

TARGET_LINK_LIBRARIES(proxylib util socketlib)
