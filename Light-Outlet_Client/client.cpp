
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include "../rc-switch/RCSwitch.h"
#include <unordered_map>


#define PORT 9996
using namespace std;
int main(int argc, char const *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    int result;
    char buffer[1024] = {0};
    int count = 1;
  
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "204.48.22.79", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    printf("About to connect \n");
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("Connected \n");
    send(sock , CONFIRM , strlen(CONFIRM) , 0 );
    printf("Confirm Sent \n");
	//sockaddr_in client_addr = new_keypad.keypad_addr;
	fd_set readset; // fd_set is a set of sockets
	/* Call select() */
	while (count) {
		do {
		FD_ZERO(&readset); // Clear an fd_set
		FD_SET(sock, &readset); // Add a descriptor to an fd_set
		result = select(sock + 1, &readset, NULL, NULL, NULL);
		} while (result == -1 && errno == EINTR);
		if (result > 0) {
		   if (FD_ISSET(sock, &readset)) {
			  /* The socket_fd has data available to be read */
			  result = recv(sock, buffer, 1024, 0);
			  if (result == 0) {
				 /* This means the other side closed the socket */
				 printf("Socket Closed\n");
				 close(sock);
				 count = 0;
			  }
			  else {
				toggle_switch(buffer);
			  }
		   }
		}
		else if (result < 0) {
		   /* An error ocurred, just print it to stdout */
		   printf("Error on select(): %s\"", strerror(errno));
		}
	}
    return 0;
}

void toggle_switch(char *buffer) 
{
	char *pch;
	int PIN = 0;
	int protocol = 0; // A value of 0 will use rc-switch's default value
    int pulseLength = 200;
    int code;
    string Str;
	
	pch = strtok(buffer,"+");
	
	if (pch != NULL) {
		auto iter = device_map.find(pch);
	
		if (iter == device_map.end())
		{
			cout << "Could Not Find That Device" << endl;
			
		} else {
			cout << iter->first << endl;
			
			
			pch = strtok(NULL,"01");
		
			if (strcmp("On",pch) == 0){
				code =  iter->second[0];
			} else {
				code =  iter->second[1];
			}
		}
	}
    
	

    if (wiringPiSetup () == -1) printf("setup error\n");
    printf("sending code[%i]\n", code);
    RCSwitch mySwitch = RCSwitch();
    if (protocol != 0) mySwitch.setProtocol(protocol);
    if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);
    mySwitch.enableTransmit(PIN);
    
    mySwitch.send(code, 24);
}
