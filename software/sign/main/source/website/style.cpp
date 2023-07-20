#include <website/http_handlers.hpp>

static constexpr auto css = "body{background:#40444b;font-family:'Sans-serif'}.center{position:absolute;top:50%;left:50%;transform:translate(-50%, -50%)}.button3D{border-radius:20px;border:none;cursor:pointer;color:#222;background:#ffe300;filter: drop-shadow(3px 3px 0 #ffb400)}.button3D:active{transform:translate(2px, 2px);filter: drop-shadow(1px 1px 0 #ffb400)}.content-container{display:block;margin:20px 3vw;padding:20px;background:#2f3136;border-radius:10px;color:white;filter: drop-shadow(5px 5px 10px #222);-moz-box-shadow:5px 5px 10px #222;-webkit-box-shadow:5px 5px 10px #222}.content-container h1{text-align:center}.content-container input[type=password],input[type=number],input[type=text]{display:block;padding:10px;color:white;border:none;border-radius:5px;background:#222;filter: drop-shadow(inset 5px 5px 10px #222);-moz-box-shadow:inset 5px 5px 10px #111;-webkit-box-shadow:inset 5px 5px 10px #111}.content-container input{height:2em;display:inline-block;margin-bottom:5px}.content-container input[type=submit]{display:inline-block;width:100%;margin-top:20px;background:#ffe300;border-radius:20px;border:none;cursor:pointer;color:#222;background:#ffe300;filter: drop-shadow(3px 3px 0 #ffb400)}.content-container input[type=submit]:active{transform:translate(2px, 2px);filter: drop-shadow(1px 1px 0 #ffb400)}";

esp_err_t style_handler(httpd_req_t *req) {
	
	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, css, HTTPD_RESP_USE_STRLEN);

	return ESP_OK;
}
