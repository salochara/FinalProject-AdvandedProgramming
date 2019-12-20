#include <stdio.h>
#include <string.h>
#include "../mongoose.h"
#define FILE_NAME "differences.txt"

#define BUFFER_SIZE 50
#define SIZE_OF_ARRAY 30

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
    //printf("%s file name\n",file_name);
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


// Function that fills an array of size SIZE_OF_ARRAY and returns a pointer to the array created.
int * fillArray()
{
    srand( (unsigned)time( NULL ) ); // for having a different seed for the random numbers.
    int static array [SIZE_OF_ARRAY];
    for (int i = 0; i < SIZE_OF_ARRAY; ++i) {
        array[i] = rand() % 100 + 1; //random numbers from range [1,100]
    }
    return array;
}

// void function -> receives as a constant the number input by the user and the array filled with random numbers.
// This function compares the inputNumber with each of the element of the array and prints the difference in a new file.
void writeToFile(const int inputNumber,const int* array)
{
    int subtraction;
    FILE * file_ptr = NULL; // A pointer to a file. Needed for handling files.
    file_ptr = fopen("./WebRoot/differences.txt", "w"); // Open file in specified directory
    if(file_ptr) // to verify if the file is open
    {
        for (int j = 0; j < SIZE_OF_ARRAY; ++j){
            subtraction = inputNumber - array[j];
            fprintf(file_ptr, "%d \n", subtraction);
        }
        fclose(file_ptr);
        printf("File successfully called: \"%s\" \n", FILE_NAME);
    }else
        printf("File not created \n");
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
            if (mg_vcmp(&hm->uri, "/pi-save") == 0)
            {
                char buffer[BUFFER_SIZE];

                /* First option for sending a response
                sprintf(buffer, "conn_id sleep:%f", result);
                mg_send_head(nc,200,strlen(buffer),"Content-Type: text/plain");
                mg_printf(nc,"%s",buffer);
                */

                /* Second option for sending HTML */
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

            }else if(mg_vcmp(&hm->uri, "/array-save") == 0)
            {
                char buffer[BUFFER_SIZE];
                int * array;
                // Get the amount of iterations to be done from the html form
                mg_get_http_var(&hm->body, "number", buffer,sizeof(buffer));
                
                // Cast what's passed into an int 
                int value = atoi(buffer);

                // Fill an array with random values
                array = fillArray();
                
                // Calculate the difference with value - array[i]
                writeToFile(value,array);

                // Send the result numbers as a string

                sprintf(buffer, "File created with name: \"%s\"", FILE_NAME);
                mg_send_head(nc,200,strlen(buffer),"Content-Type: text/plain");
                mg_printf(nc,"%s",buffer);


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


int initServer(int port){
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
    printf("while"); //todo

    while(1){
        mg_mgr_poll(&mgr,100);
    }
    mg_mgr_free(&mgr);

    return 0;
}



