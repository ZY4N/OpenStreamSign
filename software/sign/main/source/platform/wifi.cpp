
#include <platform/wifi.hpp>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include <netdb.h>
#include "nvs_flash.h"

static const char *TAG = "wifi station";

static constexpr auto WIFI_CONNECTED_BIT = BIT0;
static constexpr auto WIFI_FAIL_BIT      = BIT1;


// variables can't be caputered by lambda because of C API
static EventGroupHandle_t s_wifi_event_group;
static int maxRetries = -1;

uint32_t static_ip, static_netmask, static_gateway;

uint32_t wifi::parseIP(const char *ip) {
	return ipaddr_addr(ip);
}

static void set_static_ip(esp_netif_t *netif) {
	ESP_LOGD(TAG, "Trying to set static ip");
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }
    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = static_ip;
    ip.netmask.addr = static_netmask;
    ip.gw.addr = static_gateway;
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    ESP_LOGD(TAG, "Success to set static ip");
}

template<size_t N>
static bool copyStringToBuffer(const std::string_view& str, uint8_t (&buffer)[N]) {
	if (str.size() >= N)
		return false;

	std::copy(str.begin(), str.end(), buffer);
	buffer[str.size()] = '\0';
	
	return true;
};


static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	static int currentRetries = 0;

	 if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        set_static_ip((esp_netif_t *) arg);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (currentRetries < maxRetries) {
            esp_wifi_connect();
            currentRetries++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "static ip:" IPSTR, IP2STR(&event->ip_info.ip));
        currentRetries = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
int wifi::client::connect(
	const std::string_view& ssid, const std::string_view& password,
	uint32_t ip, uint32_t netmask, uint32_t gateway,
	int numRetries
) {
	int state = 0;

	static_ip = ip;
	static_netmask = netmask;
	static_gateway = gateway;

	s_wifi_event_group = xEventGroupCreate();

	if (esp_netif_init() != ESP_OK)
		return state;

	state++;
	// If the event loop is already created it will return 'ESP_ERR_INVALID_STATE'
	if (const auto ret = esp_event_loop_create_default(); not (ret == ESP_OK or ret == ESP_ERR_INVALID_STATE))
		return state;
	
	state++;
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    if (sta_netif == nullptr)
		return state;

	state++;
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	if (esp_wifi_init(&cfg) != ESP_OK)
		return state;
		
	maxRetries = numRetries;

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	state++;
	if (esp_event_handler_instance_register(
		WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, sta_netif, &instance_any_id
	) != ESP_OK) return state;

	state++;
	if (esp_event_handler_instance_register(
		IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, sta_netif, &instance_got_ip
	) != ESP_OK) return state;

	wifi_config_t wifi_config = {
		.sta = {
			{},						// ssid
			{},						// password
			{}, 					// scan_method
			{}, 					// bssid_set
			{}, 					// bssid
			{}, 					// channel
			{}, 					// listen_interval
			{}, 					// sort_method
			{						// threshold
				{},					// rssi
				WIFI_AUTH_WPA2_PSK  // authmode
			},
			{						// pmf_cfg
				true,		    	// capable
				false				// required
			},
			{},						// rm_enabled
			{},						// btm_enabled
			{},						// mbo_enabled
			{}						// reserved
		}
	};

	const auto copyStringToBuffer = []<size_t Size>(const std::string_view& str, uint8_t (&buffer)[Size]) {
		
		ESP_LOGI(TAG, "copyStringToBuffer actual: %d buffer: %d", str.length(), Size);
		if (str.length() >= Size)
			return false;

		std::copy(str.begin(), str.end(), buffer);
		buffer[str.size()] = '\0';
		return true;
	};

	state++;
	if (not copyStringToBuffer(ssid, wifi_config.sta.ssid))
		return state;
	
	state++;
	if (not copyStringToBuffer(password, wifi_config.sta.password))
		return state;

	state++;
	if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK)
		return state;

	state++;
	if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK)
		return state;

	state++;
	if (esp_wifi_start() != ESP_OK)
		return state;

	EventBits_t bits = xEventGroupWaitBits(
		s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY
	);

	state++;
	// The event will not be processed after unregister
	if (esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip))
		return state;

	state++;
	if (esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id))
		return state;

	vEventGroupDelete(s_wifi_event_group);

	return 70 + (bits & WIFI_CONNECTED_BIT);
}

void wifi::client::disconnect() {
	esp_wifi_stop();
}


bool wifi::ap::create(
	const std::string_view& ssid,
	const std::string_view& password,
	uint8_t channel, uint8_t maxConnections
) {
	if (esp_netif_init() != ESP_OK)
		return false;

    if (esp_event_loop_create_default() != ESP_OK)
		return false;

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg))
		return false;

    if (esp_event_handler_instance_register(
		WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL
	)) return false;

    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = static_cast<uint8_t>(ssid.length()),
            .channel = channel,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = maxConnections,
            .pmf_cfg = { .required = false }
        }
    };

	if (not copyStringToBuffer(ssid, wifi_config.ap.ssid))
		return false;

	if (not copyStringToBuffer(password, wifi_config.ap.password))
		return false;


    if (ssid.length() == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    if (esp_wifi_set_mode(WIFI_MODE_AP) != ESP_OK)
		return false;

    if (esp_wifi_set_config(WIFI_IF_AP, &wifi_config) != ESP_OK)
		return false;

    if (esp_wifi_start() != ESP_OK)
		return false;
		
	return true;
}

void wifi::ap::destroy() {
	esp_wifi_stop();
}
