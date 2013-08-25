SET( APP_SRCS
sfrp_main.cpp
http_proxy_stream_handler.cpp
options.cpp
combined_log_record.cpp
access_log.cpp
pipeline_data.cpp
pipeline_data_factory.cpp
host_rules.cpp
) 


SET_SOURCE_FILES_PROPERTIES(SVNVersion.cpp PROPERTIES GENERATED 1)


ADD_EXECUTABLE(sfrp ${APP_SRCS})

TARGET_LINK_LIBRARIES(sfrp util sfevent proxylib http socketlib event pthread ${Boost_LIBRARIES})


