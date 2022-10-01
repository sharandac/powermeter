/****************************************************************************
 *   Tu May 22 21:23:51 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <WiFi.h>
#include "wifi_config.h"
#include "wificlient.h"

wificlient_config_t::wificlient_config_t() : BaseJsonConfig( WIFICLIENT_JSON_CONFIG_FILE ) {
}

bool wificlient_config_t::onSave(JsonDocument& doc) {

    doc["hostname"] = hostname;
    doc["ssid"] = ssid;
    doc["password"] = password;
    doc["enable_softap"] = enable_softap;
    doc["softap_ssid"] = softap_ssid;
    doc["softap_password"] = softap_password;
    doc["timeout"] = timeout;
    doc["low_bandwidth"] = low_bandwidth;
    doc["low_power"] = low_power;

    return true;
}

bool wificlient_config_t::onLoad(JsonDocument& doc) {
    /*
     * make an uniqe Hostname for the SoftAp SSID
     */
    uint8_t mac[6];
    char tmp_hostname[ WIFICLIENT_MAX_TEXT_SIZE] = "";
    WiFi.macAddress( mac );
    snprintf( tmp_hostname, sizeof( tmp_hostname ), "powermeter_%02x%02x%02x", mac[3], mac[4], mac[5] );
    
    strlcpy( hostname, doc["hostname"] | tmp_hostname, sizeof( hostname ) );
    strlcpy( ssid, doc["ssid"] | "", sizeof( ssid ) );
    strlcpy( password, doc["password"] | "", sizeof( password ) );
    enable_softap = doc["enable_softap"] | true;
    strlcpy( softap_ssid, doc["softap_ssid"] | tmp_hostname, sizeof( softap_ssid ) );
    strlcpy( softap_password, doc["softap_password"] | "powermeter", sizeof( softap_password ) );
    timeout = doc["timeout"] | 15;
    low_bandwidth = doc["low_bandwidth"] | false;
    low_power = doc["low_power"] | false;
    
    return true;
}

bool wificlient_config_t::onDefault( void ) {
    /*
     * make an uniqe Hostname for the SoftAp SSID
     */
    uint8_t mac[6];
    log_i("Write first config to SPIFFS\r\n");    
    WiFi.macAddress( mac );
    snprintf( hostname, sizeof( hostname ), "powermeter_%02x%02x%02x", mac[3], mac[4], mac[5] );
    snprintf( softap_ssid, sizeof( softap_ssid ), "powermeter_%02x%02x%02x", mac[3], mac[4], mac[5] );

    return true;
}
