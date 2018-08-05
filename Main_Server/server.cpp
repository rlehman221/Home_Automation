// Server side C/C++ program
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <map>
#include <stdio.h>
#include <thread>
#include "server.h"
#include <queue>
#include <arpa/inet.h>
#include <unordered_map>

#define PORT 9996
using namespace std;
queue <int> server_queue;
int count = 0;
unordered_map<string, lock> lock_map;
unordered_map<string, int> light_map;
unordered_map<string, lock>:: iterator lock_itr;
typedef void (*Connection_Function)(int socket_value);
typedef unordered_map<std::string, Connection_Function> connection_map;
connection_map socket_interfaces;

int main(int argc, char const *argv[])
{
	struct sockaddr_in address;
    int server_fd;
    int opt = 1;
    int rc;
	char hostname[50];
	rc = gethostname(hostname,sizeof(hostname));
	if(rc == 0){
		printf("hostname = %s\n",hostname);
	}
    thread t;
	server_queue.push(1);
	create_connection(server_fd, opt, address);
	bind_socket(server_fd, address);
	socket_interfaces.insert(make_pair("keypad_code", &create_keypad_connection));
	socket_interfaces.insert(make_pair("lock_code", &create_lock_connection));
	socket_interfaces.insert(make_pair("light_code", &add_light_to_map));
	socket_interfaces.insert(make_pair("web_interface", &web_interface_thread));
	
	
	while (count < 1) {
		printf("count %d\n", count);
		int thread_type = server_queue.front();
		server_queue.pop();
		
		switch(thread_type) {
			case INITIALIZE_THREAD:
				printf("Launch Main thread\n");
				t = thread(accept_connections, ref(server_fd), ref(address), 1);
				t.join();
				count++;
				break;
			case WEB_THREAD:
				printf("This is a web thread");
				break;
			case KEYBOARD_THREAD:
				printf("This is a keyboard thread");
				break;
		}
		sleep(2);
	}

	printf("Launched from main\n");
    
    return 0;
}

void create_connection(int &server_fd, const int &opt, sockaddr_in &address)
{
	// Creating socket file descriptor
	// AF_INET (IPv4 protocol)
	// SOCK_STREAM: TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
	
}


void bind_socket(int &server_fd, sockaddr_in &address) 
{
	// Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void accept_connections(int &server_fd, sockaddr_in &address, int i) 
{
	int new_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	int counter = 0;
	while (counter < 2) {
		
		if (listen(server_fd, 3) < 0)
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}
		
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
		{
			perror("accept\n");
			exit(EXIT_FAILURE);
		}
		
		/*// Obtain IP addr. for accepted socket
		struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&address;
		struct in_addr ipAddr = pV4Addr->sin_addr;
		char str[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
		*/
		
		
		read(new_socket, buffer, 1024);
		
		cout << buffer << endl;
		auto iter = socket_interfaces.find(buffer);
		
	    if (iter == socket_interfaces.end())
	    {
	    	cout << "Could Not Find That Socket Interface" << endl;
	    } else {
			(*iter->second)(new_socket);
		}
		counter++;
	}
}

void add_light_to_map(int socket_value)
{
	light_map["Light_Map"] = socket_value;
}

void create_keypad_connection(int socket_value) 
{
	struct keypad new_keypad;
	fd_set readset;
	int result;
	char buffer[1024] = {0};
	new_keypad.socket_num = socket_value;
	//new_keypad.keypad_addr = address;
	for (int i = 0; i < KEYPAD_SETUP; i++) {
		do {
			FD_ZERO(&readset);
			FD_SET(new_keypad.socket_num, &readset);
			result = select(new_keypad.socket_num + 1, &readset, NULL, NULL, NULL);
		} while (result == -1 && errno == EINTR);
		if (result > 0) {
		   if (FD_ISSET(new_keypad.socket_num, &readset)) {
			  /* The socket_fd has data available to be read */
			  result = recv(new_keypad.socket_num, buffer, 1024, 0);
			  
			  if (result == 0) {
				 /* This means the other side closed the socket */
				 printf("Socket Closed\n");
				 close(new_keypad.socket_num);
				 break;
			  }
			  switch (i) {
				  case (0): new_keypad.my_uid = buffer;
							printf("My UID: %s\n", buffer);
							break;
				  case (1): new_keypad.lock_uid = buffer;
							printf("LOCK UID: %s\n", buffer);
							break;
			  }
		   }
		}
		else if (result < 0) {
		   /* An error ocurred, just print it to stdout */
		   printf("Error on select(): %s\"", strerror(errno));
		}
	}
	send(new_keypad.socket_num, CONFIRM , strlen(CONFIRM) , 0);
	keypad_thread(new_keypad);
}

void create_lock_connection(int socket_value) 
{
	struct lock new_lock;
	fd_set readset;
	int result;
	char buffer[1024] = {0};
	//new_lock.lock_addr = address;
	new_lock.socket_num = socket_value;
	for (int i = 0; i < LOCK_SETUP; i++) {
		do {
			FD_ZERO(&readset);
			FD_SET(new_lock.socket_num, &readset);
			result = select(new_lock.socket_num + 1, &readset, NULL, NULL, NULL);
			printf("Result: %d\n", result);
		} while (result == -1 && errno == EINTR);
		if (result > 0) {
		   if (FD_ISSET(new_lock.socket_num, &readset)) {
			  /* The socket_fd has data available to be read */
			  result = recv(new_lock.socket_num, buffer, 1024, 0);
			  
			  if (result == 0) {
				 /* This means the other side closed the socket */
				 printf("Socket Closed\n");
				 close(new_lock.socket_num);
				 break;
			  }
			  switch (i) {
				  case (0): new_lock.my_uid = buffer;
							break;
				  case (1): new_lock.device_code = buffer;
							break;
			  }
		   }
		}
		else if (result < 0) {
		   /* An error ocurred, just print it to stdout */
		   printf("Error on select(): %s\"", strerror(errno));
		}
	}
	send(new_lock.socket_num, CONFIRM , strlen(CONFIRM) , 0);
	lock_map[new_lock.my_uid] = new_lock;
}

void keypad_thread(keypad new_keypad) 
{
	int result;
	struct timeval time_out;
	//sockaddr_in client_addr = new_keypad.keypad_addr;
	char buffer[1024] = {0};
	fd_set readset; // fd_set is a set of sockets
	int counting = 1;
	printf("In Thread\n");
	/* Call select() */
	while (counting) {
		do {
		time_out.tv_sec  = 2;
		FD_ZERO(&readset); // Clear an fd_set
		FD_SET(new_keypad.socket_num, &readset); // Add a descriptor to an fd_set
		printf("Before Read\n");
		result = select(new_keypad.socket_num + 1, &readset, NULL, NULL, &time_out);
		printf("Result: %d\n", result);
		} while (result == -1 && errno == EINTR);
		if (result > 0) {
		   if (FD_ISSET(new_keypad.socket_num, &readset)) {
			  /* The socket_fd has data available to be read */
			  result = recv(new_keypad.socket_num, buffer, 1024, 0);
			  printf("%s\n", buffer);
			  if (result == 0) {
				 /* This means the other side closed the socket */
				 printf("Socket Closed\n");
				 close(new_keypad.socket_num);
				 counting = 0;
			  }
			  else {
				 printf("Othern");
			  }
		   }
		}
		else if (result < 0) {
		   /* An error ocurred, just print it to stdout */
		   printf("Error on select(): %s\n", strerror(errno));
		} 
	}
}

void web_interface_thread(int socket_value) 
{
	int result;
	struct timeval time_out;
	char buffer[1024] = {0};
	fd_set readset; // fd_set is a set of sockets
	int counting = 1;
	printf("In Thread\n");

	while (counting) {
		do {
		time_out.tv_sec  = 2;
		FD_ZERO(&readset); // Clear an fd_set
		FD_SET(socket_value, &readset); // Add a descriptor to an fd_set
		result = select(socket_value + 1, &readset, NULL, NULL, &time_out);
		} while (result == -1 && errno == EINTR);
		if (result > 0) {
		   if (FD_ISSET(socket_value, &readset)) {
			  result = recv(socket_value, buffer, 1024, 0);
			  if (result == 0) {
				 printf("Socket Closed\n");
				 close(socket_value);
				 counting = 0;
			  }
			  else {
				 send_to_switch(buffer);
			  }
		   }
		}
		else if (result < 0) {
		   printf("Error on select(): %s\n", strerror(errno));
		} 
	}
}

void send_to_switch(char *buffer)
{
	
	auto iter = light_map.find("Light_Map");
	
    if (iter == light_map.end())
    {
    	cout << "Could Not Find That Socket Interface" << endl;
    	
    } else {
		send(iter->second, buffer , strlen(buffer) , 0);
	}
}
	

