PROJECT(SWITCHFLOW)

MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

IF( ${CMAKE_BUILD_TYPE} MATCHES Debug )
  MESSAGE("Configuring Debug Build.")
  ADD_DEFINITIONS(-DDEBUG)
  LINK_DIRECTORIES(${SWITCHFLOW_SOURCE_DIR}/../thirdparty/lib/debug ${SWITCHFLOW_SOURCE_DIR}/../lib)
ELSE( ${CMAKE_BUILD_TYPE} MATCHES Debug )
  MESSAGE("Configuring Release Build.")
  LINK_DIRECTORIES(${SWITCHFLOW_SOURCE_DIR}/../thirdparty/lib/release ${SWITCHFLOW_SOURCE_DIR}/../lib)
ENDIF( ${CMAKE_BUILD_TYPE} MATCHES Debug )

IF( ${CMAKE_COMPILER_IS_GNUCXX} )
  ADD_DEFINITIONS(-static-libgcc)
  ADD_DEFINITIONS(-pthread)
  SET(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -static-libgcc")
ENDIF( ${CMAKE_COMPILER_IS_GNUCXX} )


INCLUDE_DIRECTORIES(
${SWITCHFLOW_SOURCE_DIR}/
${SWITCHFLOW_SOURCE_DIR}/../thirdparty/include
${SWITCHFLOW_SOURCE_DIR}/../thirdparty/boost
${SWITCHFLOW_SOURCE_DIR}/../thirdparty/boost-asio/include
)

SUBDIRS(util 
        event
        socketlib 
        client 
        http
	test
)

