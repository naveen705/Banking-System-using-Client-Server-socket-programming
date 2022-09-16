#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
//for takking to the server
char * talk_to_server(int sock_fd, char *str,int it)
{
//for receiving information from the server
	if(it==0)
	{
		int packets = 0;
	    int n = read(sock_fd, &packets, sizeof(int));
	    if(n <= 0) {
	        shutdown(sock_fd, SHUT_WR);
	        return NULL;
	    }
	    char *str = (char*)malloc(packets*512);
	    memset(str, 0, packets*512);
	    char *previous = str;
	    int i;
	    for(i = 0; i < packets; ++i) {
	        int n = read(sock_fd, str, 512);
	        str = str+512;
	    }
	    return previous;
	}
	//for sending information to the server
	else if(it==1)
	{
		char *s;
		int packets = (strlen(str)-1)/512 + 1;
        	int n = write(sock_fd, &packets, sizeof(int));
        	char *msg = (char*)malloc(packets*512);
        	strcpy(msg, str);
        	int i;
        	for(i = 0; i < packets; ++i) {
               int n = write(sock_fd, msg, 512);
           	msg += 512;
       	}
       	return s;
	}
}

//main function
int main(int argc,char **argv)
{
//for making connection with the server and binding
	int fd_socket,p_no;
	struct sockaddr_in address_server;
	char *msgFromServer;
    char msgToServer[256];


	fd_socket=socket(AF_INET, SOCK_STREAM, 0);
	p_no = atoi(argv[2]);
	//setting up socket address domain and port number
	memset(&address_server, 0, sizeof(address_server));  
	address_server.sin_family = AF_INET;         
    address_server.sin_port = htons(p_no);    
    inet_aton(argv[1], &address_server.sin_addr);
//connecting to the server
    connect(fd_socket, (struct sockaddr*)&address_server, sizeof(address_server));

    printf("Connection has Established.\n");
    

    while(1) {
        msgFromServer = talk_to_server(fd_socket,"",0);
        if(msgFromServer == NULL)
            break;
        if(strncmp(msgFromServer, "unauth", 6) == 0) {
            printf("Incorrect user credentials.\n");
            shutdown(fd_socket, SHUT_WR);
            break;
        }
        printf("%s\n",msgFromServer);
        free(msgFromServer);
        
        memset(msgToServer, 0, sizeof(msgToServer));
        scanf("%s", msgToServer);
        talk_to_server(fd_socket, msgToServer,1);
        if(strncmp(msgToServer, "exit", 4) == 0) {
            shutdown(fd_socket, SHUT_WR);
            break;
        }
    }

    while(1) {
        msgFromServer = talk_to_server(fd_socket,"",0);
        if(msgFromServer == NULL)
            break;
        printf("%s\n",msgFromServer);
        free(msgFromServer);
    }
    //ending 
    printf("end closed by the server.\n");
    shutdown(fd_socket, SHUT_RD);
    printf("Connection is closed gracefully.\n");
    return 0;


}
