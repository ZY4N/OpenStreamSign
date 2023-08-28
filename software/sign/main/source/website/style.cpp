#include <website/http_handlers.hpp>

static constexpr auto css = ":root{--unit-width:min(50vw, 50vh);--unit-height:min(6vw, 6vh)}body{background:#40444b;font-family:Sans-serif}.center{position:absolute;top:50%;left:50%;transform:translate(-50%,-50%)}.button3D{border-radius:20px;border:none;cursor:pointer;color:#222;background:#ffe300;filter:drop-shadow(3px 3px 0 #ffb400)}.button3D:active,.content-container input[type=submit]:active,input[type=button]:active{transform:translate(2px,2px);filter:drop-shadow(1px 1px 0 #ffb400)}.content-container{display:block;width:var(--unit-width);margin:calc(.3 * var(--unit-height)) calc(.1 * var(--unit-width));padding:calc(.3 * var(--unit-height)) calc(.1 * var(--unit-width));background:#2f3136;border-radius:10px;color:#fff;font-size:calc(.4 * var(--unit-height));filter:drop-shadow(5px 5px 10px #222);-moz-box-shadow:5px 5px 10px #222;-webkit-box-shadow:5px 5px 10px #222}.content-container h1{text-align:center;font-size:var(--unit-height)}.content-container input{width:calc(98%);height:var(--unit-height);display:inline-block;margin-bottom:5px}.content-container input[type=password],input[type=ipv4],input[type=number],input[type=text]{display:block;padding:calc(.1 * var(--unit-height));color:#fff;font-size:calc(.6 * var(--unit-height));border:none;border-radius:5px;background:#222;filter:drop-shadow(inset 5px 5px 10px #222);-moz-box-shadow:inset 5px 5px 10px #111;-webkit-box-shadow:inset 5px 5px 10px #111}.content-container input[type=submit],input[type=button]{display:inline-block;width:100%;margin-top:calc(.8 * var(--unit-height));background:#ffe300;border-radius:calc(.5 * var(--unit-height));border:none;cursor:pointer;color:#222;font-size:calc(.7 * var(--unit-height));filter:drop-shadow(3px 3px 0 #ffb400)}";

esp_err_t style_handler(httpd_req_t *req) {
	
	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, css, HTTPD_RESP_USE_STRLEN);

	return ESP_OK;
}
