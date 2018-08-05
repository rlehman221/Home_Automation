#ifndef SERVER_H
#define SERVER_H

#include <string.h>
#define INITIALIZE_THREAD (1)
#define WEB_THREAD		  (2)
#define KEYBOARD_THREAD   (3)
#define CONFIRM 		  ("success")
#define KEYPAD_SETUP	  (2)
#define LOCK_SETUP	  	  (2)
struct keypad {
	std::string my_uid;
	std::string lock_uid;
	int socket_num;
	sockaddr_in keypad_addr;
};

struct lock {
	std::string my_uid;
	std::string device_code;
	int socket_num;
	sockaddr_in lock_addr;
};


void create_connection(int &server_fd, const int &opt, sockaddr_in &address);
void bind_socket(int &server_fd, sockaddr_in &address);
void accept_connections(int &server_fd, sockaddr_in &address, int i);
void keypad_thread(keypad new_keypad);
void create_keypad_connection(int socket_value);
void create_lock_connection(int socket_value);
void web_interface_thread(int socket_value);
void add_light_to_map(int socket_value);
void send_to_switch(char *buffer);

#endif /* SERVER_H */

/*
 * RESOURCES:
 * 
 * - SELECT: https://www.binarytides.com/multiple-socket-connections-fdset-select-linux/
 * 
 * 
 * 
 * 
 */
