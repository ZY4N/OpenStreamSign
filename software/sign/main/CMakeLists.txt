idf_component_register(
	SRCS
		"source/domain_logic/sign.cpp"
		"source/domain_logic/sign_animation_controller.cpp"

		"source/platform/lwip_socket_connection.cpp"
		"source/platform/lwip_socket_acceptor.cpp"
		"source/platform/mbedtls_aes_256_engine.cpp"
		"source/platform/mbedtls_hmac_sha_512_engine.cpp"
		"source/platform/wifi_access_point_handler.cpp"
		"source/platform/wifi_client_handler.cpp"
		"source/platform/wifi_generic_handler.cpp"
		
		"source/website/done.cpp"
		"source/website/networking.cpp"
		"source/website/root.cpp"
		"source/website/script.cpp"
		"source/website/secret.cpp"
		"source/website/style.cpp"
		"source/website/wifi.cpp"

		"source/app/main_task.cpp"

		#"../../common/source/hmac_sha_512_handshake.cpp"

        "main.cpp"

	INCLUDE_DIRS
		"include"
		"source"
		"../../common/include"
		"../../common/source"

	EMBED_TXTFILES
		"include/website/certs/servercert.pem"
		"include/website/certs/prvtkey.pem"

	#REQUIRES
	#	"common"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=c++20 -Os)
