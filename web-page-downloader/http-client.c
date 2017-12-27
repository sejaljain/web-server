#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>



static void die(char *s) {
	perror(s);
	exit(1);
}



int main(int argc, char **argv) {
	if (argc != 4) { die("command line arguments"); }

	int sock;                        /* Socket descriptor */
	struct sockaddr_in servAddr; /* server address */
	unsigned short servPort;     /* server port */
	char *servIP;                    /* Server IP address (dotted quad) */
	char *path;  		/* file path */
	char *filename;
	int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() 									                                        and total bytes read */
	struct hostent *he;
	char *serverName = argv[1];
	// get server ip from server name
	if ((he = gethostbyname(serverName)) == NULL) {
		die("gethostbyname failed");
	}
	servIP = inet_ntoa(*(struct in_addr *)he->h_addr); 
	path = argv[3];
	filename = strrchr(path, '/');
	filename++;
	servPort = atoi(argv[2]);
	/* Create a reliable, stream socket using TCP */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		    die("socket() failed");
	
	/* Construct the server address structure */
	memset(&servAddr, 0, sizeof(servAddr));     /* Zero out structure */
	servAddr.sin_family      = AF_INET;             /* Internet address family */
	servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
	servAddr.sin_port        = htons(servPort); /* Server port */
	
	/* Establish the connection to the server */
	if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)							die("connect() failed");

	FILE *server = fdopen(sock, "r");
	if (server == NULL) {
		die("server file");
	}
	FILE *output = fopen(filename, "ab+");
	if (output == NULL) {
		die("output file");
	}

	char request[4096] = "";
	sprintf(request, "GET %s HTTP/1.0\r\nHost: %s:%d\r\n\r\n", path, serverName, servPort);
	if(send(sock, request, strlen(request), 0) < 0) { die("send failed"); }

	char header[4096] = "";
	fgets(header, sizeof(header), server);
	char *code = strchr(header, ' ');
	code++;
	code[3] = 0;
	
	if(strcmp(code, "200") != 0) {
		printf("%s", header);
		die("request failed");
	}


	while(fgets(header, sizeof(header), server)[0] != '\r') { }
	char response[4096] = "";
	size_t n;
	while((n = fread(response, sizeof(char), sizeof(response), server)) > 0) {
		if (fwrite(response, sizeof(char), n, output) != n) {
			die("fwrite failed");
		}
	}

	if (ferror(server)) { die("fread failed"); }
	fclose(server);
	fclose(output);

	return 0;
}	

