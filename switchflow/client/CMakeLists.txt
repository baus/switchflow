ADD_LIBRARY(client 
            client_handler.cpp)

TARGET_LINK_LIBRARIES(client sfevent util)
