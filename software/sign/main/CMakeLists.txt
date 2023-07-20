idf_component_register(
	SRCS
		"source/domain_logic/sign.cpp"
		"source/domain_logic/sign_animation_controller.cpp"

		"source/platform/lwip_socket_connection.cpp"
		"source/platform/lwip_socket_acceptor.cpp"
		"source/platform/mbedtls_aes_256_engine.cpp"
		"source/platform/mbedtls_hmac_sha_512_engine.cpp"
		"source/platform/button.c"
		"source/platform/wifi.cpp"
		
		"source/website/done.cpp"
		"source/website/networking.cpp"
		"source/website/root.cpp"
		"source/website/script.cpp"
		"source/website/secret.cpp"
		"source/website/style.cpp"
		"source/website/wifi.cpp"
		"source/app/main_task.cpp"
		"source/app/button_task.cpp"
		"source/app/setup_task.cpp"
		"source/app/sign_task.cpp"

		"../../common/source/hmac_sha_512_handshake.cpp"

        "main.cpp"

	INCLUDE_DIRS
		"include"
		"source"
		"../../common/include"
		"../../common/source"

	EMBED_TXTFILES
		"include/website/certs/servercert.pem"
		"include/website/certs/prvtkey.pem"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=c++20 -Os -fno-exceptions )
