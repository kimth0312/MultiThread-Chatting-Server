#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/shm.h>
#include <cstring>

#define PORT	8080
#define BUFFER_SIZE  1024

using namespace std;

int main() {
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		cerr << "Socket creation failed.\n" << endl;
		exit(1);
	}

	struct sockaddr_in serv;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(PORT);
	serv.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (sockaddr *)&serv, sizeof(serv)) < 0) {
		cerr << "Connection failed." << endl;
		exit(1);
	}

	char buf[BUFFER_SIZE];
	string userInput;

	do {
		cout << "> ";
		getline(cin, userInput);

		if (userInput.size() > 0) {
			int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
			if (sendResult != -1) {
				memset(buf, 0, 4096);
				int received = recv(sock, buf, 4096, 0);
				if (received > 0) {
					cout << "Server> " << string(buf, 0, received) << endl;
				}
			}
		}
	} while (userInput.size() > 0);

	close(sock);


	return 0;
}
