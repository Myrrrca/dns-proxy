#pragma once

#include <stdint.h> 
#include <arpa/inet.h>

#include "config.h"

int getdomain(char* data, char* domain_name); 

int check_domain(char* data, Config config);

void create_and_bind_server_socket(int* server_sockfd,
                      struct sockaddr_in* server_addr, const Config config);


void create_upstream_server_socket(int* upstream_server_sockfd, 
                      struct sockaddr_in* upstream_server_addr, const Config config);

void print_client_addr(uint16_t* client_port, char** client_ip, 
                              const struct sockaddr_in client_addr);
