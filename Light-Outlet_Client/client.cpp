/* client.cpp - TCP socket client that connect to a server side
 * socket and waits for commands. Once a command is received, it 
 * goes through a mapping stage to parse both the device in request
 * along with the required action. The data is then transmitted using
 * RF modules.
 * 
 * Date: June-15-2018
 * Created by: Ryan Lehman
*/

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

using namespace std;
int main(int argc, char const *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    int result;
    char buffer[1024] = {0};
    int isValid = 1;
  
	// Creates client socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error \n");
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
  
	// Connects to the server through TCP connection
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection Failed \n");
        return -1;
    }
	
	// Sends a confirmation code to server showing its ID
    send(sock , CONFIRM , strlen(CONFIRM) , 0 );
    
	fd_set readset; // Set of sockets

	while (count) {
		do {
		FD_ZERO(&readset); // Clear set
		FD_SET(sock, &readset);
		result = select(sock + 1, &readset, NULL, NULL, NULL);
		} while (result == -1 && errno == EINTR);
		if (result > 0) {
		   if (FD_ISSET(sock, &readset)) {
			  // Server has data available to be read
			  result = recv(sock, buffer, 1024, 0);
			  if (result == 0) {
				 // Server side closed the socket */
				 printf("Socket Closed \n");
				 close(sock);
				 isValid = 0;
			  } else {
				// Toggle on/off outlet based on server command
				toggle_switch(buffer);
			  }
		   }
		}
		else if (result < 0) {
		   printf("Error on select(): %s\"", strerror(errno));
		}
	}
    return 0;
}

void toggle_switch(char *buffer) 
{
	char *pch;
	int PIN = 0;
	int protocol = 0; // 0 default value
    int pulseLength = 200; // Freq. to transmit
    int code;
    string Str;
	
	// Extract device type
	pch = strtok(buffer,"+");
	
	if (pch != NULL) {
		// Check to see if device requested is valid
		auto iter = device_map.find(pch);
	
		if (iter == device_map.end())
		{
			cout << "Could Not Find That Device" << endl;
			
		} else {
			// If the device is found, check the command to execute
			pch = strtok(NULL,"01");
			
			if (strcmp("On",pch) == 0){
				code =  iter->second[0];
			} else {
				code =  iter->second[1];
			}
		}
	}
	
	// Transmit to device the servers command
    if (wiringPiSetup () == -1) printf("setup error \n");
    printf("sending code[%i] \n", code);
    RCSwitch mySwitch = RCSwitch();
    if (protocol != 0) mySwitch.setProtocol(protocol);
    if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);
    mySwitch.enableTransmit(PIN);
    mySwitch.send(code, 24);
}
