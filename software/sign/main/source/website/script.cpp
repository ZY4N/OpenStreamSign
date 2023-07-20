#include <website/http_handlers.hpp>

static constexpr auto js = "function encodeString(e,t){for(let n=0;n<e.length;n++){let o=e.charCodeAt(n);if(o<32||o>=127)return!1;t.push(o)}return!0}function encodeUnsignedShort(e,t){let n=parseInt(e);if(isNaN(n)||n<0||n>65535)return!1;let o=new ArrayBuffer(2),r=new Uint16Array(o);r[0]=n;let a=new Uint8Array(o);return t.concat(Array.from(a)),!0}function encodeIPv4(e,t){return null!=e.match(\"^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).){3}(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))$\")&&(e.split(\".\").forEach(e=>t.push(parseInt(e))),!0)}window.addEventListener(\"load\",()=>{let e={txt:encodeString,ipv4:encodeIPv4,u16:encodeUnsignedShort};for(let t of document.forms)t.addEventListener(\"submit\",n=>{n.preventDefault();let o=[];try{for(let r of t.elements){let a=e[r.dataset.t];if(a){if(!encodeString(r.name,o))throw`Cannot encode name '${r.name}' of field '${r.name}'`;if(o.push(255),!a(r.value,o))throw`Invalid value '${r.value}' in field '${r.name}'`;o.push(254)}}}catch(i){alert(i);return}o.pop();let l=new XMLHttpRequest;l.open(\"POST\",window.location.href),l.setRequestHeader(\"Content-Type\",\"application/octet-stream\"),l.onload=()=>{l.readyState===l.DONE&&(200===l.status?window.location.href=t.action:alert(\"The Backend died, try again!\"))},l.send(new Uint8Array(o))})});";

esp_err_t script_handler(httpd_req_t *req) {
	httpd_resp_set_type(req, "text/javascript");
	return httpd_resp_send(req, js, HTTPD_RESP_USE_STRLEN);
}
