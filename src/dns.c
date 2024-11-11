#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dns.h"
#include "errproc.h"

int getdomain(char* data, char* domain_name) {
  int expected_length = 0;
  int counter = 0;
  int labels = 1;
  int i = 0;

  expected_length = data[i];
  // printf("expected_length = %d\n", data[i]);

  for (int i = 1; data[i] != 0; ++i) {
      if (expected_length <= 0) {
        expected_length = data[i];
        domain_name[counter] = '.';
        ++labels;
        ++counter;
        // printf("expected_length = %d\n", data[i]);
        continue;
      }
      domain_name[counter] = data[i];
      ++counter;
      --expected_length;
      // printf("now reading %c\n", data[i]);
  }
  // printf("domain name after reading: %s\n", domain_name);
  return labels;
}  

int check_domain(char* data, Config config) {
  char domain_name[2048] = {0};
  getdomain(&data[12], domain_name);  
  printf("domain name after reading: %s\n", domain_name);
  for (int i = 0; config.domains[i] != NULL; ++i) {
    if (strcmp(domain_name, config.domains[i]) == 0) {
      // printf(" sorry bro no %s for you\n", config.domains[i]);
      printf("%s\n", config.response);
      return 1;
    }
  }
  printf("domain name %s is valid\n", domain_name);
  return 0;
  // for (int i = 0; i <= label_counter; ++i) {
  //   printf("domain label #%d after reading: %s\n", i, domain_name[i]);
  // }
}

void create_and_bind_server_socket(int* server_sockfd,
                          struct sockaddr_in* server_addr, const Config config) {
  *server_sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

  // removing 'adress already in use' error on our port
  int yes = 1;
  Setsockopt(*server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  server_addr->sin_family = AF_INET;
  server_addr->sin_port = Htons(53);
    
  Inet_aton(config.resolv_ip, &(server_addr->sin_addr));
  Bind(*server_sockfd, (struct sockaddr* )server_addr, sizeof(struct sockaddr_in));
  printf("listening on port %d\n\n", Htons(server_addr->sin_port));
}

void create_upstream_server_socket(int* upstream_server_sockfd, 
                      struct sockaddr_in* upstream_server_addr, const Config config) {
  *upstream_server_sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
  upstream_server_addr->sin_family = AF_INET;
  upstream_server_addr->sin_port = Htons(53);

  Inet_aton(config.ip, &(upstream_server_addr->sin_addr)); 
}

void print_client_addr(uint16_t* client_port, char** client_ip, 
                              const struct sockaddr_in client_addr) {
  *client_port = Ntohs(client_addr.sin_port);
  *client_ip = (char*)malloc(INET_ADDRSTRLEN);
  if (*client_ip == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
  }
  strcpy(*client_ip, Inet_ntoa(client_addr.sin_addr));
  #if DUMP
  printf("Received packet from %s:%d\n", *client_ip, *client_port);
  #endif
}

