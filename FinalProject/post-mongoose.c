#include <stdio.h>
#include <string.h>
#include "mongoose.h"

// Struct containing settings for serving hTTP requests
static struct mg_serve_http_opts s_http_server_opts;
int initServer(int port);

int main()
{
    initServer(8080);
}

void event_handler(struct mg_connection * nconnection, int event, void * p)
{
    if(event == MG_EV_HTTP_REQUEST)
    {
        // Serve html files
        mg_serve_http(nconnection,(struct http_message *)p,s_http_server_opts);
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
    struct mg_connection * nconnection;

    // Init mongoose server
    mg_mgr_init(&mgr,NULL);

    // Bind server to the nconnection
    nconnection = mg_bind(&mgr,stringPort,event_handler);

    // Setting for the server
    mg_set_protocol_http_websocket(nconnection);
    s_http_server_opts.index_files = "yes";
    s_http_server_opts.document_root = ".";


    while(1)
    {
        mg_mgr_poll(&mgr,100);
    }
    mg_mgr_free(&mgr);

    return 0;
}



