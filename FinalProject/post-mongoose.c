#include <stdio.h>
#include <string.h>
#include "mongoose.h"

#define BUFFER_SIZE 50



// Struct containing settings for serving hTTP requests
static struct mg_serve_http_opts s_http_server_opts;
int initServer(int port);

int main()
{
    initServer(8080);
}

// For uploading files. Need to compile with  -D MG_ENABLE_HTTP_STREAMING_MULTIPART
struct mg_str cb(struct mg_connection *c, struct mg_str file_name) {
    // Return the same filename. Do not actually do this except in test!
    // fname is user-controlled and needs to be sanitized.
    printf("%s file name\n",file_name);
    return file_name;
}

double computePI(char * buffer)
{
    // Approximation to pi using the following formula :
    // pi = 4 - (4/3) + (4/5) - (4/7) + (4/9) - ...
    double result = 4;
    int sign = -1;
    unsigned long int divisor = 3;

    // Casting the string received into an int
    int iterations = atoi(buffer);

    for (int i = 0; i < iterations; ++i)
    {
        result += sign * (4.0 / divisor);
        sign *= -1;
        divisor += 2;
    }
    return result;
}



static void handle_save(struct mg_connection *nc, struct http_message *hm) {
    // Get form variables and store settings values

    // Send response
    mg_http_send_redirect(nc, 302, mg_mk_str("/"), mg_mk_str(NULL));
}

void event_handler(struct mg_connection * nc, int event, void * p)
{
    struct http_message *hm = (struct http_message *) p;
    switch (event)
    {
        /*
        // For uploading files
        case MG_EV_HTTP_PART_BEGIN:
        case MG_EV_HTTP_PART_DATA:
        case MG_EV_HTTP_PART_END:
            mg_file_upload_handler(nc, event, p, cb);
            break;
        */
        case MG_EV_HTTP_REQUEST:
            if (mg_vcmp(&hm->uri, "/save") == 0)
            {
                char buffer[BUFFER_SIZE];

                // Get the amount of iterations to be done from the html form
                mg_get_http_var(&hm->body, "iterations", buffer,sizeof(buffer));

                // Use chunked encoding in order to avoid calculating Content-Length
                mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");

                // Compute PI with the amount of iterations received from the user
                double result = computePI(buffer);

                // Output JSON object which holds the result of PI
                mg_printf_http_chunk(nc, "{ \"result\": %f }", result);

                // Send empty chunk, the end of response
                mg_send_http_chunk(nc, "", 0);


            }
            // For index to show
            else{
                //printf("IN INDEX %s\n",hm->message.p); prints the whole message
                mg_serve_http(nc,(struct http_message *)p,s_http_server_opts);
                //printf("buffer %s\n",nc->recv_mbuf.buf);
            }
            break;
        case MG_EV_HTTP_REPLY:
            printf("reply case\n");
            break;
        default:
            break;
    }

}


int initServer(int port)
{
    char stringPort[10];
    sprintf(stringPort,"%d",port);
    printf("Listening on port: %s\n",stringPort);
    // Mongoose event manager
    struct mg_mgr mgr;
    // Mongoose connection
    struct mg_connection * nc;

    // Init mongoose server
    mg_mgr_init(&mgr,NULL);

    // Bind server to the nc
    nc = mg_bind(&mgr,stringPort,event_handler);

    // Setting for the server
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.index_files = "yes";
    s_http_server_opts.document_root = "./WebRoot";


    while(1)
    {
        mg_mgr_poll(&mgr,100);
    }
    mg_mgr_free(&mgr);

    return 0;
}



