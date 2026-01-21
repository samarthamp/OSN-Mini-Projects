#include "header.h"
#include "iman-17.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HOST "man.he.net"
#define PORT 443 // HTTPS port

void execute_iman(char **args) {
    if (args[1] == NULL) {
        printf("iMan: missing command argument\n");
        return;
    }

    // 1. Initialize OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        return;
    }

    // 2. Standard Socket Connection
    struct hostent *he;
    struct sockaddr_in server_addr;

    if ((he = gethostbyname(HOST)) == NULL) {
        herror("iMan: gethostbyname");
        return;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("iMan: socket");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(server_addr.sin_zero), 0, 8);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("iMan: connect");
        close(sockfd);
        return;
    }

    // 3. Bind SSL to the Socket
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);

    // 4. Perform SSL Handshake
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        // 5. Send Encrypted Request
        char request[1024];
        snprintf(request, sizeof(request), 
                 "GET /?topic=%s&section=all HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Connection: close\r\n\r\n", 
                 args[1], HOST);

        SSL_write(ssl, request, strlen(request));

        // 6. Receive Encrypted Response
        char buffer[4096];
        int bytes_received;
        int headers_parsed = 0;

        while ((bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_received] = '\0';
            
            // Basic logic to strip headers (look for double newline)
            if (!headers_parsed) {
                char *body_start = strstr(buffer, "\r\n\r\n");
                if (body_start) {
                    headers_parsed = 1;
                    printf("%s", body_start + 4);
                } else {
                     // In a real app, you'd buffer this until you find the header end
                     // For this snippet, we assume headers fit in the first chunk
                     printf("%s", buffer); 
                }
            } else {
                printf("%s", buffer);
            }
        }
    }

    // Cleanup
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
}