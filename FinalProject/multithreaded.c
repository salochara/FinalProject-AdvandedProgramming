/*
 * Mongoose: Copyright (c) 2014-2017 Cesanta Software Limited
 * Roberto Alejandro Gutiérrez Guillén		    A01019608
 * Salomón Charabati Michan			            A01022425
 * Alberto Ramos					            A01374020
 */
#include "mongoose.h"
#include "game_of_life_omp.h"

#define FILE_NAME "differences.txt"
#define BUFFER_SIZE 100
#define NUM_THREADS 4
#define SIZE_OF_ARRAY 30

typedef enum programs{PI, ARRAY, GAME_OF_LIFE} programs_t;

static sig_atomic_t s_received_signal = 0;
static unsigned long s_next_id = 0;
static struct mg_serve_http_opts s_http_server_opts;
static sock_t sock[2];
char file_name_global[MAX_STRING_SIZE];
int ompIterations = 10;


/////------ Structs ------
// For thread use
static void signal_handler(int sig_num) {
    signal(sig_num, signal_handler);
    s_received_signal = sig_num;
}

// This info is passed to the worker thread
typedef struct work_request {
    unsigned long conn_id;  // Identify the connection where to send the reply
    int type;
    int input_number;
}request_t;

// This info is passed by the worker thread to mg_broadcast
typedef struct work_result {
    unsigned long conn_id;
    int type;
    int input_number;
    double result;
}result_t;

// For uploading files. Need to compile with  -D MG_ENABLE_HTTP_STREAMING_MULTIPART
struct mg_str cb(struct mg_connection *c, struct mg_str file_name) {
    strncpy(file_name_global,file_name.p,MAX_STRING_SIZE);
    return file_name;
}

/////------ Functions for applications ------

// Function that fills an array of size SIZE_OF_ARRAY and returns a pointer to the array created.
int * fillArray(){
    srand( (unsigned)time( NULL ) ); // for having a different seed for the random numbers.
    int static array [SIZE_OF_ARRAY];
    for (int i = 0; i < SIZE_OF_ARRAY; ++i) {
        array[i] = rand() % 100 + 1; //random numbers from range [1,100]
    }
    return array;
}

// void function -> receives as a constant the number input by the user and the array filled with random numbers.
// This function compares the inputNumber with each of the element of the array and prints the difference in a new file.
void writeToFile(const int inputNumber,const int* array){
    int subtraction;
    FILE * file_ptr = NULL; // A pointer to a file. Needed for handling files.
    file_ptr = fopen("./WebRoot/differences.txt", "w");
    if(file_ptr){ // to verify if the file is open
        for (int j = 0; j < SIZE_OF_ARRAY; ++j){
            subtraction = inputNumber - array[j];
            fprintf(file_ptr, "%d \n", subtraction);
        }
        fclose(file_ptr);
        printf("File successfully called: \"%s\" \n", FILE_NAME);
    }else{
        printf("File not created \n");
    }
}

double computePI(int iterations){
    // Approximation to pi using the following formula :
    // pi = 4 - (4/3) + (4/5) - (4/7) + (4/9) - ...
    double result = 4;
    int sign = -1;
    unsigned long int divisor = 3;

    for (int i = 0; i < iterations; ++i){
        result += sign * (4.0 / divisor);
        sign *= -1;
        divisor += 2;
    }
    return result;
}

/////------ Thread functions------

// Pushes the work to broadcast for the thread, only called when result is asked,
static void result_thread(struct mg_connection *nc, int ev, void *ev_data) {
    (void) ev;
    char s[BUFFER_SIZE];
    struct mg_connection *c;

    // Cycle through out all incoming connection requests
    for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
        if (c->user_data != NULL) {
            // Put the received parameters in work result struct 
            struct work_result *res = (struct work_result *)ev_data;

            // Check if connection is where info needs to go
            if ((unsigned long)c->user_data == res->conn_id) {
                // Switch case for different programs
                switch(res->type){
                    case PI:
                        // Write result on a new page
                        sprintf(s, "conn_id:%lu Iterations:%d pi:%f", res->conn_id, res->input_number, res->result);
                        mg_send_head(c, 200, strlen(s), "Content-Type: text/plain");
                        mg_printf(c, "%s", s);
                        break;
                    case  ARRAY:
                        sprintf(s, "Conn_id:%lu Input number:%d Result file name:%s", res->conn_id, res->input_number, FILE_NAME);
                        mg_send_head(c, 200, strlen(s), "Content-Type: text/plain");
                        mg_printf(c, "%s", s);
                        break;
                    case GAME_OF_LIFE: // Redirect with results to a new page
                        sprintf(s, "Conn_id:%lu  Iterations:%d   Files successfully created from %s", res->conn_id, res->input_number, file_name_global);
                        mg_send_head(c, 200, strlen(s), "Content-Type: text/plain");
                        mg_printf(c, "%s", s);
                        break;
                    default:
                        break;
                }

            }
        }
    }
}

// Prepares the thread and calls the result thread. This one is initialized since the inception of the thread
void *worker_thread(void *param) {
    struct mg_mgr *mgr = (struct mg_mgr *) param;

    // Initialize struct to store parameters sent from main thread
    struct work_request req = {0,0,0}; // Conn id, type, iterations

    // Initialize struct to store information
    struct work_result res = {0,0,0, 0};  ;

    // Initialize variables for array
    int * array;

    pgm_t pgm_image;
    char output_file_name[MAX_STRING_SIZE];

    // Loops loking for a signal
    while (s_received_signal == 0) {
        // Read request information
        if (read(sock[1], &req, sizeof(req)) < 0){
            perror("Reading worker sock");
        }

        // Copy connection id, type and number to result struct
        res.conn_id = req.conn_id;
        res.type = req.type;
        res.input_number = req.input_number;

        switch(req.type){
            case PI:
                // Calculate pi and assign values to result struct
                res.result = computePI(req.input_number);

                break;
            case  ARRAY:

                // Fill an array with random values
                array = fillArray();

                // Calculate the difference with value - array[i] and write result in text file
                writeToFile(res.input_number,array);

                // No result

                break;
            case GAME_OF_LIFE:
                /* Does the Game Of Life code*/
                // Read the contents of a text file and store them in an image structure
                readPGMFile(file_name_global,&pgm_image);

                strncpy(output_file_name,file_name_global, MAX_STRING_SIZE);
                strip_ext(output_file_name);
                // Call the iterations of Game Of Life to be applied and write the data in the image structure into a new PGM file
                iterationsOfGameOfLife(&pgm_image, res.input_number, output_file_name);
                break;
            default:
                break;
        }
        // Calls the result thread with the struct
        mg_broadcast(mgr, result_thread, (void *)&res, sizeof(res));
    }
    return NULL;
}

/////------ Server functions------
// Main orchestrator, this reads the type of event and specifies what to do with it
static void ev_handler(struct mg_connection *nc, int event, void *ev_data) {
    // Declare message to see where browser is being redirected
    struct http_message *hm = (struct http_message *) ev_data;
    // Declare buffer t
    char buffer[BUFFER_SIZE];

    // Main switch to decide what to do, where to redirect
    switch (event) {
        case MG_EV_HTTP_PART_BEGIN:
        case MG_EV_HTTP_PART_DATA:
        case MG_EV_HTTP_PART_END:
            mg_file_upload_handler(nc, event, ev_data, cb);
            // Create a work request struct to send data to thread
            struct work_request req = {(unsigned long)nc->user_data, GAME_OF_LIFE, ompIterations};

            // Send to the thread information
            if (write(sock[0], &req, sizeof(req)) < 0){
                perror("Writing worker sock");
            }
            break;
        case MG_EV_ACCEPT:
            nc->user_data = (void *)++s_next_id;
            break;
        case MG_EV_HTTP_REQUEST: {
            if (mg_vcmp(&hm->uri, "/pi-save") == 0){ // Pi
                // Get the amount of iterations to be done from the html form
                mg_get_http_var(&hm->body, "iterations", buffer,sizeof(buffer));

                // Create a work request struct to send data to thread
                struct work_request req = {(unsigned long)nc->user_data, PI,atoi(buffer)};

                // Send to the thread information
                if (write(sock[0], &req, sizeof(req)) < 0){
                    perror("Writing worker sock");
                }
            }else if(mg_vcmp(&hm->uri, "/array-save") == 0){ // Array
                // Get number to subtract from array, put in buffer
                mg_get_http_var(&hm->body, "number", buffer,sizeof(buffer));

                // Create a work request struct to send data to thread
                struct work_request req = {(unsigned long)nc->user_data, ARRAY,atoi(buffer)};

                // Send to the thread information
                if (write(sock[0], &req, sizeof(req)) < 0){
                    perror("Writing worker sock");
                }
            }else if(mg_vcmp(&hm->uri, "/pre-omp") == 0) {
                mg_get_http_var(&hm->body, "iterations", buffer,sizeof(buffer));
                ompIterations = atoi(buffer);
                // Redirect to omp

                mg_http_send_redirect(nc, 302, mg_mk_str("/omp.html"), mg_mk_str(NULL));
            }else{
                // For index to show
                mg_serve_http(nc,(struct http_message *)ev_data,s_http_server_opts);
            }
            break;
        }
        case MG_EV_CLOSE:
            if (nc->user_data){ nc->user_data = NULL;}
            break;
        default:
            break;
    }
}

int initServer(int port){
    // Mongoose Event Manager
    struct mg_mgr mgr;
    // Mongoose connection
    struct mg_connection *nc;

    // Open sockets
    if (mg_socketpair(sock, SOCK_STREAM) == 0) {
        perror("Opening socket pair");
        exit(1);
    }

    // Signal Handlers
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // Init mongoose server
    mg_mgr_init(&mgr, NULL);

    // Bind server to the nc
    char stringPort[10];
    sprintf(stringPort,"%d",port);
    nc = mg_bind(&mgr, stringPort, ev_handler);
    if (nc == NULL) {
        printf("Failed to create listener\n");
        return 1;
    }
    // Setting for the server
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.index_files = "yes";
    s_http_server_opts.document_root = "./WebRoot";// Serve current directory

    // Create server threads
    for (int i = 0; i < NUM_THREADS; i++) {
        mg_start_thread(worker_thread, &mgr);
    }

    printf("Started on port %d\n", port);

    // Checks connections for IO readiness
    while (s_received_signal == 0) {
        mg_mgr_poll(&mgr, 200);
    }

    // Free the memory for mongoose server
    mg_mgr_free(&mgr);

    // Close sockets
    closesocket(sock[0]);
    closesocket(sock[1]);

    return 0;
}

int main() {
    initServer(8080);

    return 0;
}



