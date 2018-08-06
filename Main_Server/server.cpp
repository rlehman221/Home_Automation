/* server.cpp - TCP socket server to allow devices and interfaces 
 * to connect based on custom protocol. Accepted connections are 
 * mapped/stored for later execution or opened as a new thread awaiting
 * User input. Conforms to a blocking protocol and should be held on a 
 * static IP. 
 * 
 * Date: June-04-2018
 * Created by: Ryan Lehman
*/

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

using namespace std;
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
    thread t;
    
    // Create/Bind TCP Socket
	create_connection(server_fd, opt, address);
	bind_socket(server_fd, address);
	
	// Setup known devices/interfaces
	socket_interfaces.insert(make_pair("keypad_device", &create_keypad_connection));
	socket_interfaces.insert(make_pair("lock_device", &create_lock_connection));
	socket_interfaces.insert(make_pair("light_code", &add_light_to_map));
	socket_interfaces.insert(make_pair("web_interface", &web_interface_thread));
	
	// Open a new thread to start accepting connections
	t = thread(accept_connections, ref(server_fd), ref(address));
	t.join();

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
      
    // Allow reuse of local addresses/Ports
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
	// Attaching socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void accept_connections(int &server_fd, sockaddr_in &address) 
{
	int new_socket;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *data;
	
	while (1) {
		
		// Listen for incoming connections with a queue of 3
		if (listen(server_fd, 3) < 0)
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}
		
		// Accept the incoming connection
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
		{
			perror("accept\n");
			exit(EXIT_FAILURE);
		}
		
		// Read the type of device
		read(new_socket, buffer, 1024);
		data = strtok(buffer,"#");
		
		// Check to see if connected device is apart of system
		auto iter = socket_interfaces.find(data);
		
	    if (iter == socket_interfaces.end())
	    {
	    	cout << "Could Not Find That Socket Interface" << endl;
	    	close(new_socket);
	    } else {
			(*iter->second)(new_socket);
		}
	}
}

void add_light_to_map(int socket_value)
{
	// Adds a light connected device to the system
	light_map["Light_Map"] = socket_value;
}

void create_keypad_connection(int socket_value) 
{
	struct keypad new_keypad;
	fd_set readset;
	int result;
	char buffer[1024] = {0};
	thread f;
	char *data;
	new_keypad.socket_num = socket_value;
	
	// Constructs a keypad
	for (int i = 0; i < KEYPAD_SETUP; i++) {
		do {
			FD_ZERO(&readset);
			FD_SET(new_keypad.socket_num, &readset);
			// Check to see if keypad is sending data
			result = select(new_keypad.socket_num + 1, &readset, NULL, NULL, NULL);
		} while (result == -1 && errno == EINTR);
		
		// Keypad has sent data
		if (result > 0) {
		   if (FD_ISSET(new_keypad.socket_num, &readset)) {
			  // The socket has data available to be read
			  result = recv(new_keypad.socket_num, buffer, 1024, 0);
			  if (result == 0) {
				 // Keypad has closed itself
				 printf("Socket Closed\n");
				 close(new_keypad.socket_num);
				 break;
			  }
			  /* Using a custom protocol, the keypad is constructed
			   * based on a certain loop ordering that both client
			   * and server react to
			  */ 
			  data = strtok(buffer,"#");
			  switch (i) {
				  case (0): new_keypad.my_uid = data;
							printf("My UID: %s\n", data);
							break;
				  case (1): new_keypad.lock_uid = data;
							printf("LOCK UID: %s\n", data);
							break;
			  }
		   }
		}
		else if (result < 0) {
		   printf("Error on select(): %s\"", strerror(errno));
		}
	}
	// Sends a success message to client confirming keypad was created
	send(new_keypad.socket_num, CONFIRM , strlen(CONFIRM) , 0);
	
	// Opens a new thread to listen for the keypad client
	f = thread(keypad_thread, new_keypad);
	f.detach();
}

void create_lock_connection(int socket_value) 
{
	struct lock new_lock;
	fd_set readset;
	int result;
	char buffer[1024] = {0};
	char *data;
	new_lock.socket_num = socket_value;
	
	// Constructs a lock
	for (int i = 0; i < LOCK_SETUP; i++) {
		do {
			FD_ZERO(&readset);
			FD_SET(new_lock.socket_num, &readset);
			// Check to see if lock is sending data
			result = select(new_lock.socket_num + 1, &readset, NULL, NULL, NULL);
		} while (result == -1 && errno == EINTR);
		
		// Lock has sent data
		if (result > 0) {
		   if (FD_ISSET(new_lock.socket_num, &readset)) {
			  // The socket has data available to be read
			  result = recv(new_lock.socket_num, buffer, 1024, 0);
			  
			  if (result == 0) {
				 // Lock has closed itself
				 printf("Socket Closed\n");
				 close(new_lock.socket_num);
				 break;
			  }
			  /* Using a custom protocol, the lock is constructed
			   * based on a certain loop ordering that both client
			   * and server react to
			  */ 
			  data = strtok(buffer,"#");
			  switch (i) {
				  case (0): new_lock.my_uid = data;
							printf("My UID: %s\n", data);
							break;
				  case (1): new_lock.device_code = data;
							printf("LOCK Code: %s\n", data);
							break;
			  }
		   }
		}
		else if (result < 0) {
		   printf("Error on select(): %s\"", strerror(errno));
		}
	}
	// Sends a success message to client confirming lock was created
	send(new_lock.socket_num, CONFIRM , strlen(CONFIRM) , 0);
	
	//Add the created lock to the system
	lock_map[new_lock.my_uid] = new_lock;
}

void keypad_thread(keypad new_keypad) 
{
	int result;
	int isValid = 1;
	struct timeval time_out;
	char buffer[1024] = {0};
	fd_set readset; // Set of sockets

	while (isValid) {
		do {
			time_out.tv_sec  = 2;
			FD_ZERO(&readset); // Clear the set
			FD_SET(new_keypad.socket_num, &readset);
			// Check to see if the keypad is sending data with a timed mark
			result = select(new_keypad.socket_num + 1, &readset, NULL, NULL, &time_out);
		} while (result == -1 && errno == EINTR);
		
		// Keypad has sent data
		if (result > 0) {
		   if (FD_ISSET(new_keypad.socket_num, &readset)) {
			  // The socket has data available to be read
			  result = recv(new_keypad.socket_num, buffer, 1024, 0);
			  printf("%s\n", buffer);
			  if (result == 0) {
				 // Closed the socket
				 printf("Socket Closed\n");
				 close(new_keypad.socket_num);
				 isValid = 0;
			  }
			  else {
				 // Calls a function to unlock/lock 
				 lock_interaction(new_keypad, buffer);
			  }
		   }
		}
		else if (result < 0) {
		   printf("Error on select(): %s\n", strerror(errno));
		} 
	}
}

void web_interface_thread(int socket_value) 
{
	int result;
	int isValid;
	struct timeval time_out;
	char buffer[1024] = {0};
	fd_set readset; // set of sockets

	while (isValid) {
		do {
		time_out.tv_sec  = 2;
		FD_ZERO(&readset); // Clear set
		FD_SET(socket_value, &readset);
		// Check to see if the web interface is sending data with a timed mark
		result = select(socket_value + 1, &readset, NULL, NULL, &time_out);
		} while (result == -1 && errno == EINTR);
		
		// interface has sent data
		if (result > 0) {
		   // The socket has data available to be read
		   if (FD_ISSET(socket_value, &readset)) {
			   
			  result = recv(socket_value, buffer, 1024, 0);
			  if (result == 0) {
				 // Closed the socket
				 printf("Socket Closed\n");
				 close(socket_value);
				 isValid = 0;
			  }
			  else {
				 // Calls a function to send on/off message to switch
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
	// Accepts a command from the web interface and passes it
	// to the corresponding switch
	auto iter = light_map.find("Light_Map");
	
    if (iter == light_map.end())
    {
    	cout << "Could Not Find That Switch Interface" << endl;
    } else {
		send(iter->second, buffer , strlen(buffer) , 0);
	}
}

void lock_interaction(keypad lock_controller, char *input_code)
{
	auto iter = lock_map.find(lock_controller.lock_uid);
	int result;
	char buffer[1024] = {0};
	char *data;
	
	// Finds corresponding lock connected with keypad
    if (iter == lock_map.end())
    {
    	cout << "Lock is not connected" << endl;
    } else {
		// Compares user input keypad code to actual lock code
		if ( ((iter->second).device_code) == input_code) {
			send((iter->second).socket_num, LOCK_SEND_REQUEST , strlen(LOCK_SEND_REQUEST) , 0);
			// Sends a confirm request to lock to perform action
			result = recv((iter->second).socket_num, buffer, 1024, 0);
			
			// Socket connection is closed
			if (result == 0) {
				 printf("Socket Closed\n");
				 close((iter->second).socket_num);
			} else {
				// Prints the state of the lock
				data = strtok(buffer,"#");
				cout << "Device is: " << data << endl;
			}
			
		} else {
			cout << "Code from keypad is invalid" << endl;
		}
	}
}
	

