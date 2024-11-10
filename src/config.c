#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "ini.h"

int handler(void* proxy, const char* section, const char* name,
                   const char* value) {
  Config* pconfig = (Config*)proxy;

  #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

  if (MATCH("proxy", "ip")) {
    pconfig->ip = strdup(value);
  } 
  else if (MATCH("proxy", "domain")) {
    static int domain_count = 0;

    /* works ones for every single domain found */
    pconfig->domains[domain_count] = strdup(value);
    ++domain_count;
  } 
  else if (MATCH("proxy", "response")) {
    pconfig->response = strdup(value);
  } 
  else if (MATCH("proxy", "resolv_ip")) {
    pconfig->resolv_ip = strdup(value);
  }
  else {
    return 0;
  }
  return 1;
}

void load_config(Config* config) {
  /* initilizing config struct with defaults */
  config->ip = NULL;
  for (int i = 0; i < MAX_DOMAINS; ++i) {
    config->domains[i] = NULL;
  }
  config->response = NULL;
  config->resolv_ip = NULL;

  if (ini_parse("./config/config.ini", handler, config) < 0) {
    fprintf(stderr, "cant load config.ini\n");
    exit(1);
  }
  if (config->ip == NULL || config->response == NULL || config->resolv_ip == NULL) {
    fprintf(stderr, "config is not valid. check documentation and try again\n");
    exit(1);
  }

  // printing config info after succsesful loading
  printf("config loaded:\n----------------\n ip = %s\n", config->ip);
  for (int i = 0; config->domains[i] != NULL; ++i) {
    printf(" domain = %s\n", config->domains[i]);
  }
  printf(" response = %s\n", config->response);
  printf(" resolv_ip = %s\n----------------\n", config->resolv_ip);
}
