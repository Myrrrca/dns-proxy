#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../lib/inih/ini.h"
#include "../lib/errproc/errproc.h"

#define BUFF_SIZE 512
#define MAX_DOMAINS 2048

typedef struct {
  const char* ip;
  const char* domains[MAX_DOMAINS];
  const char* response;
  const char* resolv_ip;
} Config;

static int handler(void* proxy, const char* section, const char* name,
                   const char* value)
{
  Config* pconfig = (Config*)proxy;

  #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

  if (MATCH("proxy", "ip")) {
    pconfig->ip = strdup(value);
  } else if (MATCH("proxy", "domain")) {
    static int domain_count = 0;

    /* works ones for every single domain found */
    pconfig->domains[domain_count] = strdup(value);
    ++domain_count;
  } else if (MATCH("proxy", "response")) {
    pconfig->response = strdup(value);
  } else if (MATCH("proxy", "resolv_ip")) {
    pconfig->resolv_ip = strdup(value);
  }
  else {
    return 0;
  }
  return 1;
}

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

int main() 
{
  Config config;

  /* initilizing config strcut with defaults */
  config.ip = NULL;
  for (int i = 0; i < MAX_DOMAINS; ++i) {
    config.domains[i] = NULL;
  }
  config.response = NULL;
  config.resolv_ip = NULL;

  if (ini_parse("./config/config.ini", handler, &config) < 0) {
    printf("cant load config.ini\n");
    return 1;
  }
  if (config.ip == NULL || config.response == NULL || config.resolv_ip == NULL) {
    printf("config is not valid. check documentation and try again\n");
    return 1;
  }

  /* printing config info after succsesful loading */
  printf("config loaded:\n----------------\n ip = %s\n", config.ip);
  for (int i = 0; config.domains[i] != NULL; ++i) {
    printf(" domain = %s\n", config.domains[i]);
  }
  printf(" response = %s\n", config.response);
  printf(" resolv_ip = %s\n----------------\n", config.resolv_ip);


  /* starting network */
  int server_sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in server_addr = {
    AF_INET,
    Htons(53),
    0
  };
  Inet_aton(config.resolv_ip, &(server_addr.sin_addr));
  Bind(server_sockfd, (struct sockaddr* )&server_addr, sizeof(struct sockaddr_in));
  printf("listening on port %d\n\n", Htons(server_addr.sin_port));

  struct sockaddr_in client_addr;
  unsigned char data[BUFF_SIZE] = {0};
  socklen_t addrlen = sizeof(struct sockaddr_in);

  while (1) {
    int nbytes = Recvfrom(server_sockfd, (char* )data, BUFF_SIZE, 0, (struct sockaddr* )&client_addr, &addrlen); 
    printf("--------------------------------\n");

    // Print client address
    unsigned short client_port = Ntohs(client_addr.sin_port);
    char* client_ipp = (char*)malloc(INET_ADDRSTRLEN);
    if (client_ipp == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(client_ipp, Inet_ntoa(client_addr.sin_addr));
    printf("Received packet from %s:%d\n", client_ipp, client_port);

    char domain_name[2048] = {0};
    getdomain(&data[12], domain_name);  
    int domain_name_length = strlen(domain_name);

    int offset = domain_name_length + 2 + 13 + 13;
    printf("offset = %d\n", offset);
    unsigned short data_count = 0;
    if (data[offset] != 0) {
      int tmp = data[offset];
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
      printf("%02X ", (unsigned char)data_send[i]);
    }
    printf("\nlength of query = %d\n", data_count);
    int res = check_domain(data_send, config);
    if (res) {
      continue;
    }

    int upstream_server_sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in upstream_server_addr = {
      AF_INET,
      Htons(53),
      0
    };
    Inet_aton(config.ip, &(upstream_server_addr.sin_addr)); 
    
    nbytes = Sendto(upstream_server_sockfd, data_send, data_count, 0, 
                    (struct sockaddr* )&upstream_server_addr, addrlen);
    if (nbytes == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

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
    printf("data[offset] = %02X\n", (unsigned char)data[offset]);
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
    Inet_aton(client_ipp, &(client_addr2.sin_addr)); 
    printf("sending response to %s:%d\n", inet_ntoa(client_addr2.sin_addr), 
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
