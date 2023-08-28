target_sources(${CMAKE_PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/source/platform/asio_socket_connection.cpp
	${CMAKE_CURRENT_LIST_DIR}/source/platform/openssl_aes_256_engine.cpp
	${CMAKE_CURRENT_LIST_DIR}/source/platform/openssl_hmac_sha_512_engine.cpp
		${CMAKE_CURRENT_LIST_DIR}/source/app.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_CURRENT_LIST_DIR}/lib
  ${CMAKE_CURRENT_LIST_DIR}/source
)
