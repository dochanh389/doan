#include "http_server_app.h"
/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include <esp_http_server.h>

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static const char *TAG = "eHTTP_SERVER";
static   httpd_handle_t server = NULL;
static httpd_req_t *REG;
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end"); 

static http_get_callback_t http_get_bmp180_callback = NULL;
/* An HTTP POST handler */
static esp_err_t data_post_handler(httpd_req_t *req)
{
    char buf[100];
    /* Read the data for the request */
     httpd_req_recv(req, buf,req->content_len);
     printf("data: %s\n" ,buf);
      // End response
     httpd_resp_send_chunk(req, NULL, 0);
     return ESP_OK;                         
}

static const httpd_uri_t post_data = {
    .uri       = "/data",
    .method    = HTTP_POST,
    .handler   = data_post_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};
void bmp180_response(char *data, int len){
    httpd_resp_send(REG, data ,len);
}


/* An HTTP GET handler */
static esp_err_t bmp180_get_handler(httpd_req_t *req)
{
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    //const char* resp_str = (const char*) "hello world";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req,(const char*) index_html_start , index_html_end - index_html_start);
    return ESP_OK;
}

static const httpd_uri_t get_bmp180 = {
    .uri       = "/bmp180",
    .method    = HTTP_GET,
    .handler   = bmp180_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

/* An HTTP GET handler */
static esp_err_t data_get_handler(httpd_req_t *req)
{
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    //const char* resp_str = (const char*) "{\"temperature\": \"27.3\",\"presura\": \"30\"}";
    //httpd_resp_set_type(req, "text/html");
    //httpd_resp_send(req, resp_str , strlen(resp_str));
    REG = req;
    //http_get_bmp180_callback();
    http_get_bmp180_callback();
    return ESP_OK;
}

static const httpd_uri_t get_data_bmp180 = {
    .uri       = "/getdatabmp180",
    .method    = HTTP_GET,
    .handler   = data_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

// /* An HTTP POST handler */
// static esp_err_t echo_post_handler(httpd_req_t *req)
// {
//     char buf[100];
//     int ret, remaining = req->content_len;

//     while (remaining > 0) {
//         /* Read the data for the request */
//         if ((ret = httpd_req_recv(req, buf,
//                         MIN(remaining, sizeof(buf)))) <= 0) {
//             if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
//                 /* Retry receiving if timeout occurred */
//                 continue;
//             }
//             return ESP_FAIL;
//         }

//         /* Send back the same data */
//         httpd_resp_send_chunk(req, buf, ret);
//         remaining -= ret;

//         /* Log data received */
//         ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
//         ESP_LOGI(TAG, "%.*s", ret, buf);
//         ESP_LOGI(TAG, "====================================");
//     }

//     // End response
//     httpd_resp_send_chunk(req, NULL, 0);
//     return ESP_OK;
// }

// static const httpd_uri_t echo = {
//     .uri       = "/echo",
//     .method    = HTTP_POST,
//     .handler   = echo_post_handler,
//     .user_ctx  = NULL
// };

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/bmp180", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/bmp180 URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } 
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}
 void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &get_bmp180);
        httpd_register_uri_handler(server, &get_data_bmp180);
        httpd_register_uri_handler(server, &post_data);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
       // httpd_register_uri_handler(server, &echo);
    }
    else{
    ESP_LOGI(TAG, "Error starting server!");
    }
}

 void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}
void http_set_callback_bmp180(void *cb){
    http_get_bmp180_callback = cb;
};

