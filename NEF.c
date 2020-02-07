#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include "NEF_error.h"

/**
 *  init_nef
 *  
 *  Sets up the server socket, binds it to the local address
 *  and then listens on port 666 for incoming connections
 * 
 */
void init_nef(error_t *err, char *debugging_enabled) {
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

    address.sa_data[0] = 0x02;
    address.sa_data[1] = 0x9A;

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
        err->message = "Could not listen on address 0.0.0.0:666";
        err->err_code = ERR_COULD_NOT_LISTEN;
        return;
    }

    err->server = server_socket;
}

/**
 *  Starts the NEF service, so we listen for a client
 *  and execute the command it sends
 * 
 *  May return on error, the error handling is done in
 *  the function calling.
 * 
 */ 
void start_nef(error_t *err, char *debugging_enabled) {
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
        } else if(!memcmp(client_command, NEF_COMMAND_SEND)) { // Somebody wants to send data into a file on the server
            printf("Receiving from client...\n");

            char *file_name = client_command + 11;
            client_command += 11;
            while(*client_command && (*client_command) != ' ' && (*client_command) != '\n') ++client_command;
            client_command[0] = 0;

            if((*file_name == '/' || strstr(file_name, "/../")) && !debugging_enabled[1]) {
                err->err_code = ERR_PATH_TRAVERSAL_DETECTED;
                err->message = "Path traversal detected\n";
                free(client_command);
                return;
            }

            char *data = (char*) malloc(1024);
            printf("Writing to file: %s\n", file_name);
            FILE *f = fopen(file_name, "wb");
            do {
                num_read = read(client_socket, data, 1024);
                fwrite(data, 1, num_read, f);
            } while(num_read > 0);
            fclose(f);

            free(data);
        }else if(!memcmp(client_command, NEF_COMMAND_APPEND)) { // Somebody wants to append to a file
            printf("Receiving from client...\n");

            char *file_name = client_command + 11;
            client_command += 11;
            while(*client_command && (*client_command) != ' ' && (*client_command) != '\n') ++client_command;
            client_command[0] = 0;

            if((*file_name == '/' || strstr(file_name, "/../")) && !debugging_enabled[1]) {
                err->err_code = ERR_PATH_TRAVERSAL_DETECTED;
                err->message = "Path traversal detected\n";
                free(client_command);
                return;
            }

            char *data = (char*) malloc(1024);

            FILE *f = fopen(file_name, "ab");
            
            if(!f) {
                err->err_code = ERR_FILE_NOT_FOUND;
                err->message = "File was not found\n";
                free(client_command);
                return;
            } else printf("Writing to file: %s\n", file_name);

            do {
                num_read = read(client_socket, data, 1024);
                fwrite(data, 1, num_read, f);
            } while(num_read > 0);
            fclose(f);

            free(data);
        } else if(!memcmp(client_command, NEF_COMMAND_RECV)) { // The client wants to receive something from the uploaded files
            printf("Sending to client...\n");

            char *file_name = client_command + 13;
            client_command += 13;
            while(*client_command && (*client_command) != ' ' && (*client_command) != '\n') ++client_command;
            client_command[0] = 0;

            if((*file_name == '/' || strstr(file_name, "/../")) && !debugging_enabled[1]) {
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
            } else printf("Reading from file: %s\n", file_name);

            fseek(f, 0, SEEK_END);
            size_t s = ftell(f);
            fseek(f, 0, SEEK_SET);

            char *data = (char*) malloc(s + 1);
            fread(data, 1, s, f);
            data[s] = 0;

            fclose(f);
            

            size_t num_written = 0;
            do {
                size_t to_write = min(1024, s - num_written);
                size_t written = write(client_socket, data + num_written, to_write);
                num_written += written;
            } while(num_written < s);

            free(data);
        } else { // Command was not found, so handle the error
            err->err_code = ERR_COMMAND_NOT_FOUND;
            int msg_len = strlen(client_command) + 19;
            char *msg = (char*) malloc(msg_len);
            memcpy(msg, "Command not found: ", 19);
            memcpy(msg + 19, client_command, strlen(client_command));
            err->message = msg;
            close(client_socket);
            return;
        }

        close(client_socket);
    }

    free(client_command);
}

void end_nef(error_t *err) {
    close(err->server); // Cleanup
}

void run_nef(char *debugging_enabled) { // Do all the steps needed to run the NEF service 2.0
    error_t err = { 0, NULL };

    if(debugging_enabled[0]) printf("Enabled debugging...\n");
    if(debugging_enabled[1]) printf("Enabled path traversal\n");
    if(debugging_enabled[2]) printf("Enabled exit on client error\n");
    if(debugging_enabled[3]) printf("Enabled exit on server error\n");

    printf("%.08X\n", *(uint32_t*) debugging_enabled);

    init_nef(&err, debugging_enabled);
    CHECK_ERROR(err);

    start_nef(&err, debugging_enabled);
    CHECK_ERROR(err);

    end_nef(&err);
    CHECK_ERROR(err);
}

int main(int argc, char **args) {
    char *debugging_enabled = (char*) malloc(4);

    for(int a = 1; a < argc; a++) {
        debugging_enabled[0] = (bool) strstr(args[a], "debugging-enabled=true");
        debugging_enabled[1] = (bool) strstr(args[a], "path-traversal-enabled=true");
        debugging_enabled[2] = (bool) strstr(args[a], "kill-client-errors=true");
        debugging_enabled[3] = (bool) strstr(args[a], "kill-server-errors=true");
    }

    while(true) { // Auto restart the service... Only exit if the error should kill the process
        run_nef(debugging_enabled);
    }

    free(debugging_enabled);

    return 0;
}