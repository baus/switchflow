SET( APP_SRCS
sfrp_main.cpp
host_map.cpp
http_proxy_stream_handler.cpp
options.cpp
combined_log_record.cpp
access_log.cpp
pipeline_data.cpp
pipeline_data_factory.cpp
SVNVersion.cpp
) 


SET_SOURCE_FILES_PROPERTIES(SVNVersion.cpp PROPERTIES GENERATED 1)

ADD_CUSTOM_COMMAND(OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/SVNVersion.cpp
                   COMMAND ${BASH}
                   ARGS ${CMAKE_CURRENT_SOURCE_DIR}/gensvnversion.sh
		   COMMENT building SVNVersion.cpp
		   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/SVNVersion.h)

ADD_EXECUTABLE(sfrp ${APP_SRCS})

TARGET_LINK_LIBRARIES(sfrp util sfevent proxylib http socketlib event boost_program_options pthread)


