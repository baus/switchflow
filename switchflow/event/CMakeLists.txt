ADD_LIBRARY(sfevent
event.cpp
poller.cpp
)

TARGET_LINK_LIBRARIES(sfevent util event)
