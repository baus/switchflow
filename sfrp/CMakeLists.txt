PROJECT(SFRP)

MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")


IF( ${CMAKE_COMPILER_IS_GNUCXX} )
  ADD_DEFINITIONS(-pthread)
ENDIF( ${CMAKE_COMPILER_IS_GNUCXX} )

INCLUDE_DIRECTORIES(
${SFRP_SOURCE_DIR}
${SFRP_SOURCE_DIR}/../switchflow
)


SUBDIRS(${SFRP_SOURCE_DIR}/../switchflow/http ${SFRP_SOURCE_DIR}/../switchflow/event ${SFRP_SOURCE_DIR}/../switchflow/socketlib ${SFRP_SOURCE_DIR}/../switchflow/util proxylib app)
