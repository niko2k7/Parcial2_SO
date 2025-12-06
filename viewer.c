#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#define PORT 3535
#define BACKLOG 4


int main() {
    struct sockaddr_in server, client;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    char buffer[256];

    // Validar error
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT); // htons para el endianismo del puerto
    server.sin_addr.s_addr = INADDR_ANY;  
    bzero(&(server.sin_zero), 8);

    int r = bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)); // Añadir en el cast 'struct'
    // Validar si error
    if(r == -1){
        perror("Error server bind");
    }

    r = listen(fd, BACKLOG);
    // Validar si error
    int size = sizeof(struct sockaddr_in);
    int fd2 = accept(fd, (struct sockaddr *)&client, &size); // Nuevo socket -> Nuevo descriptor -> Nuevo socket
    // Nota: fork(), crea un hijo para que atienda el cliente, y el padre quede esperando en el accept. En el caso de este código, se crea un modelo de un solo cliente
    r = send(fd2, "Conexión realizada correctamente", 33, 0);
    r = recv(fd2, buffer, sizeof(buffer), 0);

    printf("%s", buffer);
    close(fd2);
    close(fd);
    return 0;
}