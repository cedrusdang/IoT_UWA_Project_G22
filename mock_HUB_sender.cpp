#include <iostream>
#include <string>
#include <curl/curl.h> 
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Callback function to handle the response data
size_t write_callback(void *contents, size_t size, size_t nmemb, std::string *s) {
    size_t total_size = size * nmemb;
    s->append((char*)contents, total_size);
    return total_size;
}

void send_gps_data(double longitude, double latitude, const std::string& api_gateway_url) {
    CURL *curl;
    CURLcode res;
    std::string read_buffer;

    curl = curl_easy_init();
    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, api_gateway_url.c_str());

        // Set the request method to POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Prepare the JSON data
        json data;
        data["longitude"] = longitude;
        data["latitude"] = latitude;
        std::string post_data = data.dump();

        // Set the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

        // Set the content type header
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "Failed to send GPS data. Error: " << curl_easy_strerror(res) << std::endl;
        } else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            if (response_code >= 200 && response_code < 300) { // Check for successful status codes
                std::cout << "GPS data sent successfully!" << std::endl;
                if (!read_buffer.empty()) {
                    std::cout << "API Gateway response: " << read_buffer << std::endl;
                }
            } else {
                std::cerr << "Failed to send GPS data. HTTP status code: " << response_code << std::endl;
            }
        }

        // Cleanup
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

int main() {
    std::string api_gateway_url = "https://fnh2s8vbnl.execute-api.ap-southeast-2.amazonaws.com/receiver"; 
    send_gps_data(-122.3321, 47.6062, api_gateway_url);
    return 0;
}
