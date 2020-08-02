#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 8080
#define QUEUE 20

using namespace std;

int CreateSocket();
void AcceptLoop(int sockfd);
void HandleMessage(int sockfd);

int main() {
	int sock;
	sock = CreateSocket();
	AcceptLoop(sock);

	return 0;
}

int CreateSocket() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "Invalid Socket." << endl;
		return -1;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (sockaddr*)&server, sizeof(server)) == -1) {
		cerr << "Bind error." << endl;
		exit(1);
	}

	if (listen(sockfd, QUEUE) == -1) {
		cerr << "Listen error." << endl;
		exit(1);
	}

	return sockfd;
}

void AcceptLoop(int sockfd) {
	int conn;
	struct sockaddr_in client;
	socklen_t length = sizeof(client);
	
	while (true) {
		conn = accept(sockfd, (sockaddr *)&client, &length);

		if (conn == -1) {
			cerr << "Connection failed." << endl;
			exit(1);
		}

		char host[NI_MAXHOST];
		char service[NI_MAXHOST];

		memset(host, 0, NI_MAXHOST);
		memset(service, 0, NI_MAXHOST);

		if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
			cout << host << " connected on port " << service << endl;
		}
		else {
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			cout << host << " connected on port " << ntohs(client.sin_port) << endl;
		}
		HandleMessage(conn);

	}
}

void HandleMessage(int sockfd) {
	char buf[4096];

	while (true) {
		memset(buf, 0, 4096);

		int received = recv(sockfd, buf, 4096, 0);
		if (received == -1) {
			cerr << "Receiving failed." << endl;
			break;
		}

		if (received == 0) {
			cout << "Client disconnected." << endl;
			break;
		}

		if (received > 0) {
			cout << "Client > " << string(buf, 0, received) << endl;
		}

		send(sockfd, buf, received + 1, 0);
	}

	close(sockfd);
}
/*
	char buffer[1024];

	while(1) {
		memset(buffer, 0, sizeof(buffer));
		int len = recv(conn, buffer, sizeof(buffer), 0);

		if (len == -1) {
			cerr << "Receiving failed." << endl;
			break;
		}

		if (len == 0) {
			cout << "Client disconnected." << endl;
			break;
		}

		if (strcmp(buffer, "exit\n") == 0)
			break;

		cout << buffer;
		send(conn, buffer, len, 0);
	}
	close(conn);
	close(sockfd);
	*/

