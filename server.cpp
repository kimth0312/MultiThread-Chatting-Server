#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <mutex>
#include <thread>
#include <vector>

#define PORT 8080
#define QUEUE 20
#define MAX_CLIENT 10

using namespace std;
using std::thread;

int CreateSocket();
void AcceptLoop(int sockfd);
void HandleMessage(int sockfd);
void decreaseSock(int sockfd);

int client_count = 0;
int client_socks[MAX_CLIENT];
mutex m;

int main() {
	int sock;

	vector<thread> t;

	sock = CreateSocket();

	for (int i = 0; i < MAX_CLIENT; i++)
		t.push_back(thread(AcceptLoop, sock));

	for (int i = 0; i < MAX_CLIENT; i++)
		t[i].join();

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

		m.lock();
		client_socks[client_count++] = conn;
		m.unlock();

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
			decreaseSock(sockfd);
			break;
		}

		if (received == 0) {
			cout << "Client disconnected." << endl;
			decreaseSock(sockfd);
			break;
		}

		if (received > 0) {
			cout << "Client > " << string(buf, 0, received) << endl;
		}

		m.lock();
		for (int i = 0; i < client_count; i++)
			send(client_socks[i], buf, received + 1, 0);
		m.unlock();
	}
}

void decreaseSock(int sockfd) {
	m.lock();
	for (int i = 0; i < client_count; i++) {
		if (sockfd == client_socks[i]) {
			while (i++ < client_count-1)
				client_socks[i] = client_socks[i+1];
			break;
		}
	}
	client_count--;
	m.unlock();

	close(sockfd);
}
