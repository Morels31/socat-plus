#include <poll.h>
#include <unistd.h>
#include <openssl/ssl.h>

#include "socket_utils.h"

void forward(SSL *ssl_socket, int socket_fd)
{
        struct pollfd fds[2];
        char buffer[BUFFER_SIZE + 1];
        int ret;
        int bytes_read;
        int written;
        int tot_written;

        fds[0].fd = SSL_get_fd(ssl_socket);
        fds[0].events = POLLIN;
        fds[1].fd = socket_fd;
        fds[1].events = POLLIN;

        while (1) {
                ret = poll(fds, 2, TIMEOUT_MS);

                if (ret <= 0){
                        if(ret < 0){
                                perror("[!] Error: poll() failed.\n");
                        } else {
                                perror("[*] Warning: a connection timed out.\n");
                        }
                        break;
                }

                if (fds[0].revents & (POLLIN | POLLHUP | POLLERR)) {

                        if (!(fds[0].revents & POLLIN))
                                break;

                        bytes_read = SSL_read(ssl_socket, buffer, BUFFER_SIZE);

                        if (bytes_read <= 0)
                                break;

                        tot_written = 0;
                        while (tot_written < bytes_read) {
                                written = write(socket_fd, buffer + tot_written, bytes_read - tot_written);
                                if (written <= 0){
                                        perror("[!] Error: write() failed.\n");
                                        return;
                                }
                                tot_written += written;
                        }
                }

                if (fds[1].revents & (POLLIN | POLLHUP | POLLERR)) {

                        if (!(fds[1].revents & POLLIN))
                                break;

                        bytes_read = read(socket_fd, buffer, BUFFER_SIZE);

                        if (bytes_read <= 0)
                                break;

                        tot_written = 0;
                        while (tot_written < bytes_read) {
                                written = SSL_write(ssl_socket, buffer + tot_written, bytes_read - tot_written);
                                if (written <= 0){
                                        perror("[!] Error: write() failed.\n");
                                        return;
                                }
                                tot_written += written;
                        }
                }
        }
}
