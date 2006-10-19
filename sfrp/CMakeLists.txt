PROJECT(SFRP)

MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")


IF( ${CMAKE_BUILD_TYPE} MATCHES Debug )
  MESSAGE("Configuring Debug Build.")
  ADD_DEFINITIONS(-DDEBUG)
  LINK_DIRECTORIES(${SFRP_SOURCE_DIR}/../thirdparty/lib/debug ${SFRP_SOURCE_DIR}/../lib)
ELSE( ${CMAKE_BUILD_TYPE} MATCHES Debug )
  MESSAGE("Configuring Release Build.")
  LINK_DIRECTORIES(${SFRP_SOURCE_DIR}/../thirdparty/lib/release ${SFRP_SOURCE_DIR}/../lib)
ENDIF( ${CMAKE_BUILD_TYPE} MATCHES Debug )

IF( ${CMAKE_COMPILER_IS_GNUCXX} )
  ADD_DEFINITIONS(-static-libgcc)
  ADD_DEFINITIONS(-pthread)
  SET(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -static-libgcc")
ENDIF( ${CMAKE_COMPILER_IS_GNUCXX} )

INCLUDE_DIRECTORIES(
${SFRP_SOURCE_DIR}
${SFRP_SOURCE_DIR}/../switchflow
${SFRP_SOURCE_DIR}/../thirdparty/include
${SFRP_SOURCE_DIR}/../thirdparty/boost
${SFRP_SOURCE_DIR}/../thirdparty/boost-asio/include
)


SUBDIRS(../switchflow proxylib app)
