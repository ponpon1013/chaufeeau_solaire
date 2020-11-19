/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <sys/param.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_vfs.h"
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <spiffs_utils.h>
#include <esp_http_server.h>


//#define SPIFFS_WIFI "storage_wifi" given in protocol_examples_common.h
#define SPIFFS_WEB "storage"
//#define PATH_WIFI "/wifi" given in protocol_examples_common.h
#define PATH_WEB "/web"

#define MAX_LENGTH_BUFFER_HTTP_ANSWER 128
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)
/* Scratch buffer size */
#define SCRATCH_BUFSIZE  8192

static const char *TAG="APP";



bool parseRequest(const char *reference_uri, const char *uri_to_match, size_t match_upto){
    ESP_LOGI(TAG,"reference_uri:%s",reference_uri);
    ESP_LOGI(TAG,"uri_to_match:%s",uri_to_match);
    char* fin=(char *) malloc( FILENAME_LENGHT );
    char* debut=(char *) malloc( FILENAME_LENGHT );
    char* findDebut=(char *) malloc( FILENAME_LENGHT );
    char* findFin=(char *) malloc( FILENAME_LENGHT );
    fin=strchr(reference_uri,'*');
    if (fin!=NULL){
        ESP_LOGI(TAG,"fin:%s",fin);
        ESP_LOGI(TAG,"strlen(reference_uri):%i",strlen(reference_uri));
        ESP_LOGI(TAG,"tsrlen(fin):%i",strlen(fin));
        int sizeDebut=strlen(reference_uri)-strlen(fin)-1;
        if (sizeDebut!=0){
            strncpy(debut,reference_uri,sizeDebut);
            ESP_LOGI(TAG,"debut:%s",debut);
            findDebut=strstr(uri_to_match,debut);
            ESP_LOGI(TAG,"findDebut:%s",findDebut!=NULL?findDebut:"not found");
            findFin=strstr(uri_to_match+strlen(findDebut),fin+1);
            ESP_LOGI(TAG,"findFin:%s",findFin!=NULL?findFin:"not found");
            return (findDebut!=NULL && findFin!=NULL);
        }else{
            findFin=strstr(uri_to_match,fin+1);
            ESP_LOGI(TAG,"findFin:%s",findFin!=NULL?findFin:"not found");
            return (findDebut!=NULL && findFin!=NULL);
        }
    }
    else
    {
        return (strcmp(reference_uri,uri_to_match)==0);
    }
}

esp_err_t send_ressource(char* toSend,httpd_req_t *req){
    //on récupere le nom du fichier
    FILE *f=(FILE *) malloc( sizeof(FILE));
    // on ouvre le fichier en lecture
    f = fopen(toSend, "r");
    if ( f == NULL ) {
        ESP_LOGE(TAG,"Cannot open file %s\n",toSend);
        httpd_resp_send_404(req);
        fclose(f);
        return ESP_ERR_NOT_FOUND;
    }
    else
    //on l'envoie au client
    {
        char * buffer = (char *) malloc( MAX_LENGTH_BUFFER_HTTP_ANSWER );
        while ( ! feof(f) ) {
            fgets( buffer, MAX_LENGTH_BUFFER_HTTP_ANSWER, f );
            httpd_resp_send_chunk(req,buffer,strlen(buffer));
            /*ESP_LOGI(TAG,"sent:%s",buffer);
            ESP_LOGI(TAG,"size sent:%i",strlen(buffer));*/
        }
        httpd_resp_send_chunk(req,NULL,0);
        free(buffer);
        fclose(f);
    }
    return ESP_OK;
}

//HTML handler
esp_err_t html_get_handler(httpd_req_t *req)
{
    //on récupere le nom du fichier
    //FILE *f=(FILE *) malloc( sizeof(FILE));
    char* filename=malloc(sizeof(char) * FILENAME_LENGHT);
    strcpy(filename,PATH_WEB);
    //strcat(filename,"/");
    strcat(filename,req->uri);
    httpd_resp_set_type(req,"text/html");
    return send_ressource(filename,req);
}

//CSS handler
esp_err_t css_get_handler(httpd_req_t *req)
{
    //on récupere le nom du fichier
    //FILE *f=(FILE *) malloc( sizeof(FILE));
    char* filename=malloc(sizeof(char) * FILENAME_LENGHT);
    strcpy(filename,PATH_WEB);
    //strcat(filename,"/");
    strcat(filename,req->uri);
    // on met en type text/mime
    httpd_resp_set_type(req,"text/css");
    return send_ressource(filename,req);
}


/* An HTTP GET handler */
esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

httpd_uri_t html = {
    .uri       = "/*.html",
    .method    = HTTP_GET,
    .handler   = html_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};

httpd_uri_t css = {
    .uri       = "/*.css",
    .method    = HTTP_GET,
    .handler   = css_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};

/* An HTTP POST handler */
esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/* An HTTP POST handler */
esp_err_t upload_post_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;
    char buffer[256];
    
    //const char *filename = get_path_from_uri(filepath,((struct file_server_data *)req->user_ctx)->base_path,req->uri + sizeof("/upload") - 1, sizeof(filepath));
    httpd_req_recv(req,buffer,req->content_len);
    ESP_LOGI(TAG,"buffer:%s",buffer);
    
    httpd_resp_send(req,NULL,0);
    return ESP_OK;
}



/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */


/*httpd_uri_t ctrl = {
    .uri       = "/ctrl",
    .method    = HTTP_PUT,
    .handler   = ctrl_put_handler,
    .user_ctx  = NULL
};*/

httpd_handle_t start_webserver(void)
{
    //static struct file_server_data *server_data = NULL;

    /* Allocate memory for server data */
    /*server_data = calloc(1, sizeof(struct file_server_data));
    if (!server_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
        return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->base_path,PATH_WEB,sizeof(server_data->base_path));*/
    
    httpd_uri_t upload = {
        .uri       = "/upload",
        .method    = HTTP_POST,
        .handler   = upload_post_handler,
        .user_ctx  = NULL
    };
    
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn=&parseRequest;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &html);
        httpd_register_uri_handler(server, &css);
        httpd_register_uri_handler(server, &upload);
        //httpd_register_uri_handler(server, &ctrl);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}



static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void app_main()
{
    connected_tab *connected_AP=os_zalloc(sizeof(wifi_config_t));
    connected_AP->size=0;
    
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(spiffs_utils_init(SPIFFS_WIFI,PATH_WIFI));
    ESP_ERROR_CHECK(spiffs_utils_init(SPIFFS_WEB,PATH_WEB));

    /*char* filename=malloc(sizeof(char) * FILENAME_LENGHT);
    strcpy(filename,PATH_WEB);
    strcat(filename,"/");
    strcat(filename,"index.html");
    char *string= "test html";
    FILE *f=(FILE *) malloc( sizeof(FILE));
    f = fopen(filename, "w");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file for wrinting");
        }
        else{
            ESP_LOGI(TAG, "string created:%s",string);
            fprintf(f, "%s",string);
            ESP_LOGI(TAG, "File written");
        }
        fclose(f);*/

    httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STOP, &disconnect_handler, &server));
    ESP_ERROR_CHECK(example_connect(connected_AP));

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    

    server = start_webserver();
}
