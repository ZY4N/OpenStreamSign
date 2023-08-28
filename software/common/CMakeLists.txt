set(COMMON_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/source/hmac_sha_512_handshake.cpp
)
set(COMMON_INCLUDE_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_CURRENT_LIST_DIR}/source
)

if(DEFINED IDF_PATH)
  idf_component_register(SRCS ${COMMON_SRCS} INCLUDE_DIRS ${COMMON_INCLUDE_DIRS})
else()
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${COMMON_SRCS})
  target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${COMMON_INCLUDE_DIRS})
endif()
