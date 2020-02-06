#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include "NEF_error.h"

void init_nef(error_t *err) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1) {
        err->message = "Could not create Server Socket\n";
        err->err_code = ERR_COULD_NOT_CREATE_SOCKET;
        return;
    }

    int val = 1;
    int ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if(server_socket == -1) {
        err->message = "Could not create Server Socket\n";
        err->err_code = ERR_COULD_NOT_CREATE_SOCKET;
        return;
    }

    struct sockaddr address;
    address.sa_family = AF_INET;

    address.sa_data[0] = 0x1f;
    address.sa_data[1] = 0x90;

    address.sa_data[2] = 0;
    address.sa_data[3] = 0;
    address.sa_data[4] = 0;
    address.sa_data[5] = 0;

    int addrlen = sizeof(address);

    ret = bind(server_socket, &address, addrlen);
    if(ret == -1) {
        err->message = "Could not bind Server Socket\n";
        err->err_code = ERR_COULD_NOT_BIND_SOCKET;
        return;
    }

    ret = listen(server_socket, 5);
    if(ret == -1) {
        err->message = "Could not listen on address 0.0.0.0:8080";
        err->err_code = ERR_COULD_NOT_LISTEN;
        return;
    }

    err->server = server_socket;
}

void start_nef(error_t *err) {
    struct sockaddr address;
    int addrlen, num_read;
    char *client_command = (char*) malloc(100);


    while(true) {
        addrlen = sizeof(address);
        int client_socket = accept(err->server, &address, &addrlen);
        if(client_socket == -1) {
            int e = errno;
            printf("Errno: %d\n", e);

            err->message = "Could not accept Client Socket\n";
            err->err_code = ERR_COULD_NOT_ACCEPT_CLIENT;
            return;
        }

        num_read = read(client_socket, client_command, 99);
        client_command[num_read] = 0;

        if(!memcmp(client_command, NEF_COMMAND_HELP)) {
            write(client_socket, "Help for the NEF service:\n\tNEF_HELP: display this information\n\tNEF_UPLOAD <filename> <contents>: Upload a file from your system\n\tNEF_DOWNLOAD <path>: Download a file to your system", 181);
        } else if(!memcmp(client_command, NEF_COMMAND_SEND)) {
            printf("Receiving from client...\n");

            char *file_name = client_command + 11;
            while(*client_command && (*client_command) != ' ') ++client_command;
            client_command[0] = 0;

            if(strstr(file_name, "/../")) {
                err->err_code = ERR_PATH_TRAVERSAL_DETECTED;
                err->message = "Path traversal detected\n";
                free(client_command);
                return;
            }

            char *data = (char*) malloc(1024);

            FILE *f = fopen(file_name, "wb");
            do {
                num_read = read(client_socket, data, 1024);
                fwrite(data, 1, num_read, f);
            } while(num_read > 0);
            fclose(f);

            free(data);
        } else if(!memcmp(client_command, NEF_COMMAND_RECV)) {
            printf("Sending to client...\n");

            char *file_name = client_command + 13;
            while(*client_command && (*client_command) != ' ') ++client_command;
            client_command[0] = 0;

            if(strstr(file_name, "/../")) {
                err->err_code = ERR_PATH_TRAVERSAL_DETECTED;
                err->message = "Path traversal detected\n";
                free(client_command);
                return;
            }

            printf("Sending: %s\n", file_name);

            FILE *f = fopen(file_name, "rb");

            if(!f) {
                err->err_code = ERR_FILE_NOT_FOUND;
                err->message = "File was not found\n";
                free(client_command);
                return;
            }

            fseek(f, 0, SEEK_END);
            size_t s = ftell(f);
            fseek(f, 0, SEEK_SET);

            char *data = (char*) malloc(s + 1);
            fread(data, 1, s, f);
            data[s] = 0;

            fclose(f);
            

            size_t num_written = 0;
            do {
                size_t written = write(client_socket, data, 1024);
                num_written += written;

            } while(num_written < s);

            free(data);
        }



        close(client_socket);
    }

    free(client_command);
}

void end_nef(error_t *err) {
    close(err->server);
}

int main(int argc, char **args) {
    while(true) { // Auto restart
        error_t err = { 0, NULL };
        init_nef(&err);
        CHECK_ERROR(err);

        start_nef(&err);
        CHECK_ERROR(err);

        end_nef(&err);
        CHECK_ERROR(err);
    }

    return 0;
}