#pragma once

// max amount domain names for blacklist
#define MAX_DOMAINS 2048

// struct for config
typedef struct {
  const char* ip;
  const char* domains[MAX_DOMAINS];
  const char* response;
  const char* resolv_ip;
} Config;

// handle (parce) data from .ini config 
int handler(void* proxy, const char* section, const char* name,
                   const char* value);

// load config from file
void load_config(Config* config);
