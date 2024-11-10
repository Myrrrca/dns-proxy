#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFF_SIZE 512

#include "errproc.h"
#include "config.h"
#include "dns.h"

int main() {
  Config config;
  load_config(&config);

  int server_sockfd;
  struct sockaddr_in server_addr;
  create_and_bind_server_socket(&server_sockfd, &server_addr, config);

  struct sockaddr_in client_addr;
  unsigned char data[BUFF_SIZE] = {0};
  socklen_t addrlen = sizeof(struct sockaddr_in);

  // event loop
  while (1) {
    int nbytes = Recvfrom(server_sockfd, (char* )data, BUFF_SIZE, 0,
                          (struct sockaddr* )&client_addr, &addrlen); 
    printf("--------------------------------\n");

    uint16_t client_port;
    char* client_ip;
    print_client_addr(&client_port, &client_ip, client_addr);

    // handling query from app
    char domain_name[2048] = {0};
    getdomain(&data[12], domain_name);  
    int domain_name_length = strlen(domain_name);

    size_t offset = domain_name_length + 2 + 13 + 13;
    printf("offset = %zu\n", offset);
    uint16_t data_count = 0;
    if (data[offset] != 0) {
      int32_t tmp = data[offset];
      data_count = tmp + offset + 1;
    }
    else {
      data_count = offset + 1;
    }

    printf("data[offset] = %d\n", data[offset]);
    printf("data_count = %d\n", data_count);
    printf("data = ");

    unsigned char data_send[data_count];
    for (int i = 0; i < data_count; ++i) {
      data_send[i] = data[i];
      printf("%02X ", data_send[i]);
    }
    printf("\nlength of query = %d\n", data_count);
    int res = check_domain(data_send, config);
    if (res) {
      continue;
    }

    int upstream_server_sockfd;
    struct sockaddr_in upstream_server_addr;
    create_upstream_server_socket(&upstream_server_sockfd, &upstream_server_addr,
                                  config);

    nbytes = Sendto(upstream_server_sockfd, data_send, data_count, 0, 
                    (struct sockaddr* )&upstream_server_addr, addrlen);
    if (nbytes == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
    // query sent to upstream server

    memset(&data, 0, BUFF_SIZE);
    nbytes = Recvfrom(upstream_server_sockfd, &data, BUFF_SIZE, 0, 
                      (struct sockaddr* )&client_addr, &addrlen); 
    printf("\nwe received data from upstream server\n");

    memset(domain_name, 0, 2048);
    int labels = getdomain(&data[12], domain_name);  
    printf("labels = %d\n", labels);
    domain_name_length = strlen(domain_name);
    printf("domain name: %s\n", domain_name);
    printf("length of domain name: %d\n", domain_name_length);

    offset = domain_name_length + 2 + 13 + 14;
    data_count = 0;
    printf("offset before search = %d\n", offset);
    printf("data[offset] before search = %d\n", data[offset]);

    char in_auth = 0;
    char in_add_rec = 0;
    if (data[offset] != 0) {
      while (data[offset] != 0) {
        int tmp = data[offset];
        if (data[tmp + offset + 2] == 0 && data[tmp + offset + 3] == 2) {
          printf("we are leaving anwsers into auth\n");
          in_auth = 1;
          break;
        } else if (data[tmp + offset + 3] == 41) {
          printf("we are leaving anwsers into add_rec\n");
          in_add_rec = 1;
          break;

        }
        offset += tmp + 12;
        printf("offset during search = %d\n", offset);
        printf("data[offset] during search = %d\n", data[offset]);
        printf("checking if offset %d == 0\n", offset - 11);
      }
    }
    else {
      data_count = offset + 1;
    }
    if (in_auth == 1) {
      int tmp = data[offset];
      offset += tmp + 11; 
      printf("offset in auth = %d\n", offset);
      printf("data[offset] in auth = %d\n", data[offset]);
      while (data[offset] != 0) {
        tmp = data[offset];
        if (data[tmp + offset + 1] == 192) {
          printf("we are leaving auth\n");
          in_add_rec = 1;
          break;
        }
        tmp = data[offset];
        offset += tmp + 11;
        printf("offset in auth = %d\n", offset);
        printf("data[offset] in auth = %d\n", data[offset]);
      }
    }
    if (in_add_rec == 1) {
      int tmp = data[offset];
      offset += tmp + 11; 
      printf("offset in add_rec = %d\n", offset);
      printf("data[offset] in add_rec = %d\n", data[offset]);
      while (data[offset] != 0) {
        tmp = data[offset];
        if (data[tmp + offset + 1] != 192) {
          printf("we are done\n");
          break;
        }
        tmp = data[offset];
        offset += tmp + 12;
        printf("offset in add_rec = %d\n", offset);
        printf("data[offset] in add_rec = %d\n", data[offset]);
      }
      // offset += tmp;
    }
    else {
      offset -= 11;
    }
    data_count = offset + 1; 

    printf("offset = %d\n", offset);
    printf("data[offset] = %02X\n", data[offset]);
    printf("data_count = %d\n", data_count);
    printf("data = ");

    printf("data from %s = ", Inet_ntoa(client_addr.sin_addr));

    char data_send2[data_count];
    for (int i = 0; i < data_count; ++i) {
      data_send2[i] = data[i];
      printf("%02X ", (unsigned char)data_send2[i]);
    }
    printf("\nlength of response = %d\n", data_count);

    struct sockaddr_in client_addr2 = {
      AF_INET,
      Htons(client_port),
      0
    };
    Inet_aton(client_ip, &(client_addr2.sin_addr)); 
    printf("sending response to %s:%d\n", Inet_ntoa(client_addr2.sin_addr), 
           Htons(client_addr2.sin_port));

    Sendto(server_sockfd, data_send2, data_count, 0, 
           (struct sockaddr* )&client_addr2, addrlen);

    memset(&data, 0, BUFF_SIZE);
    memset(&data_send, 0, data_count);
    memset(&data_send2, 0, data_count);
    printf("waiting for new dns query...\n");
    printf("--------------------------------\n\n");
  }

  return 0;
}
