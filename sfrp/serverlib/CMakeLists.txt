ADD_LIBRARY(serverlib 
new_connection_handler.cpp
i_connection_handler.cpp
)

TARGET_LINK_LIBRARIES(serverlib util socketlib)
