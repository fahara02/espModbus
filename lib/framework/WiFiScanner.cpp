/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <WiFiScanner.h>

WiFiScanner::WiFiScanner(AsyncWebServer *server, SecurityManager *securityManager)
{
    server->on(SCAN_NETWORKS_SERVICE_PATH,
               HTTP_GET,
               securityManager->wrapRequest(std::bind(&WiFiScanner::scanNetworks, this, std::placeholders::_1),
                                            AuthenticationPredicates::IS_ADMIN));
    server->on(LIST_NETWORKS_SERVICE_PATH,
               HTTP_GET,
               securityManager->wrapRequest(std::bind(&WiFiScanner::listNetworks, this, std::placeholders::_1),
                                            AuthenticationPredicates::IS_ADMIN));
};

void WiFiScanner::scanNetworks(AsyncWebServerRequest *request)
{
    if (WiFi.scanComplete() != -1)
    {
        WiFi.scanDelete();
        WiFi.scanNetworks(true);
    }
    request->send(202);
}

void WiFiScanner::listNetworks(AsyncWebServerRequest *request)
{
    int numNetworks = WiFi.scanComplete();
    if (numNetworks > -1)
    {
        AsyncJsonResponse *response = new AsyncJsonResponse(false, MAX_WIFI_SCANNER_SIZE);
        JsonObject root = response->getRoot();
        JsonArray networks = root.createNestedArray("networks");
        for (int i = 0; i < numNetworks; i++)
        {
            JsonObject network = networks.createNestedObject();
            network["rssi"] = WiFi.RSSI(i);
            network["ssid"] = WiFi.SSID(i);
            network["bssid"] = WiFi.BSSIDstr(i);
            network["channel"] = WiFi.channel(i);
            network["encryption_type"] = (uint8_t)WiFi.encryptionType(i);
        }
        response->setLength();
        request->send(response);
    }
    else if (numNetworks == -1)
    {
        request->send(202);
    }
    else
    {
        scanNetworks(request);
    }
}