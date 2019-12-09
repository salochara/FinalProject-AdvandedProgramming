/*
 * Copyright (c) 2014-2017 Cesanta Software Limited
 * All rights reserved
 */
#include "mongoose.h"

#define BUFFER_SIZE 50
#define NUM_THREADS 4

static sig_atomic_t s_received_signal = 0;
// static const int NUM_THREADS = 5;
static unsigned long s_next_id = 0;
static struct mg_serve_http_opts s_http_server_opts;
static sock_t sock[2];

// For thread use
static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);
  s_received_signal = sig_num;
}

// This info is passed to the worker thread
struct work_request {
  unsigned long conn_id;  // Identify the connection where to send the reply
  int iterations;
};

// This info is passed by the worker thread to mg_broadcast
struct work_result {
  unsigned long conn_id;
  int sleep_time;
  double pi;
};

double computePI(int iterations){
    // Approximation to pi using the following formula :
    // pi = 4 - (4/3) + (4/5) - (4/7) + (4/9) - ...
    double result = 4;
    int sign = -1;
    unsigned long int divisor = 3;

    for (int i = 0; i < iterations; ++i)
    {
        result += sign * (4.0 / divisor);
        sign *= -1;
        divisor += 2;
    }
    return result;
}

// Pushes the work to broadcast for the thread, only called when result is asked,
static void result_thread(struct mg_connection *nc, int ev, void *ev_data) {
  printf("result thread\n");

  (void) ev;
  char s[32];
  struct mg_connection *c;
  
  for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
    if (c->user_data != NULL) {
      // Put the received parameters in work result struct 
      struct work_result *res = (struct work_result *)ev_data;

      if ((unsigned long)c->user_data == res->conn_id) {
        
        sprintf(s, "conn_id:%lu sleep:%d pi:%f", res->conn_id, res->sleep_time, res->pi);
        mg_send_head(c, 200, strlen(s), "Content-Type: text/plain");
        mg_printf(c, "%s", s);
      }
    }
  }
}

// Prepares the thread and calls the result thread. This one is initialized since the inception of the thread
void *worker_thread(void *param) {
  printf("worker thread\n");
  struct mg_mgr *mgr = (struct mg_mgr *) param;
  
  // Initialize struct to store parametrs sent from main thread
  struct work_request req = {0,0};
  
  // Loops loking for a signal
  while (s_received_signal == 0) {
    if (read(sock[1], &req, sizeof(req)) < 0){
      perror("Reading worker sock");
    }
    int r = rand() % 10;
    //sleep(r);
    //mg_get_http_var(&hm->body, "iterations", buffer,sizeof(buffer));
    double result = computePI(req.iterations);

    // Initialize struct to store information
    struct work_result res = {req.conn_id, r, result};
    printf("broadcast\n");
    // Calls the result thread
    mg_broadcast(mgr, result_thread, (void *)&res, sizeof(res));
  }
  return NULL;
}
/*
static void handle_save(struct mg_connection *nc, struct http_message *hm) {
    // Get form variables and store settings values

    // Send response
    mg_http_send_redirect(nc, 302, mg_mk_str("/"), mg_mk_str(NULL));
}*/

static void ev_handler(struct mg_connection *nc, int event, void *ev_data) {
  (void) nc;
  (void) ev_data; 
  struct http_message *hm = (struct http_message *) ev_data;
  char buffer[BUFFER_SIZE];

  switch (event) {
    case MG_EV_ACCEPT:
      printf("Accept\n");

      nc->user_data = (void *)++s_next_id;
      break;
    case MG_EV_HTTP_REQUEST: {
      printf("Request\n");

      if (mg_vcmp(&hm->uri, "/save") == 0){
        // Get the amount of iterations to be done from the html form
        mg_get_http_var(&hm->body, "iterations", buffer,sizeof(buffer));
        printf("ITerations, %s\n",buffer);
        // Create a work request struct to send data to thread
        struct work_request req = {(unsigned long)nc->user_data, atoi(buffer)};

        // Send to the thread information
        if (write(sock[0], &req, sizeof(req)) < 0){
          perror("Writing worker sock");
        }
        
      }else{// For index to show
        printf("else\n");
        //printf("IN INDEX %s\n",hm->message.p); prints the whole message
        mg_serve_http(nc,(struct http_message *)ev_data,s_http_server_opts);
        //printf("buffer %s\n",nc->recv_mbuf.buf);
      }
      break; 
    }
    case MG_EV_HTTP_REPLY:
      printf("reply case\n");
      break;
    case MG_EV_CLOSE: {
        if (nc->user_data) nc->user_data = NULL;
      }
  }
}

int initServer(int port){
  // Mongoose Event Manager
  struct mg_mgr mgr;
  // Mongoose connection
  struct mg_connection *nc;

  
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

  // Create server
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



