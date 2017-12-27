#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "mylist.h"
#include "mdb.h"

#define KEYMAX  5


static void die(const char *s) {
	perror(s);
	exit(1);
}

int main(int argc, char **argv) {
	// ignore SIGPIPE so that we don't terminate when we call
	// // send() on a disconnected socket.
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) { die("signal() failed"); }
	
	assert(argc == 3); // check for one db file name

	int servSock;
	int clntSock;
	struct sockaddr_in servAddr;
	struct sockaddr_in clntAddr;
	unsigned int clntLen;
	unsigned short servPort = atoi(argv[2]);
	
	servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(servSock < 0) { die("socket() failed"); }

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(servPort);
	
	if(bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		die("bind() failed");
	}

	if(listen(servSock, 5) < 0) {
		die("listen() failed");
	}
	while(1) {
	
		while(1) {
			clntLen = sizeof(clntAddr);

			if((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntLen)) < 0) {
				die("accept() failed");
			}
			char *clientIP = inet_ntoa(clntAddr.sin_addr);
			printf("\nconnection started from: %s\n", clientIP);
			FILE *input = fdopen(clntSock, "r");
			char *filename = argv[1];
			FILE *fp = fopen(filename, "rb");
			if(fp == NULL) { die(filename); }

			struct List list;
			initList(&list); 

			int loaded = loadmdb(fp, &list);
			if (loaded < 0) { die("loadmdb"); }

			fclose(fp);

			char line[1000] = "";
			char key[KEYMAX + 1] = "";
			while(fgets(line, sizeof(line), input) != NULL)  {
					strncpy(key, line, sizeof(key) - 1);
					key[sizeof(key) - 1] = '\0';

					size_t last = strlen(key) - 1;
					if(key[last] == '\n') { key[last] = '\0'; }

					struct Node *node = list.head;
					int recNo = 1;
					char buffer[100] = "";
					while(node) {
						struct MdbRec *rec = node->data;
						if(strstr(rec->name, key) || strstr(rec->msg, key)) {
							size_t length = snprintf(buffer, 100, "%4d: {%s} said {%s}\n", recNo, rec->name, rec->msg);
							send(clntSock, buffer, length, 0);
						}
						node = node->next;
						recNo++;
					}
					send(clntSock, "\r\n", strlen("\r\n"), 0);

			}
			printf("connection terminated from: %s\n", clientIP);
			fclose(input);
			freemdb(&list);
		}
	}

	return 0;
}
