#include <unordered_map>
#include <string>
#include <vector>

#define CONFIRM 		  ("light_code")
#define PORT 9996
using namespace std;

unordered_map<string, vector<int> > device_map ({
										{ "My_Room", {5330227, 5330236}}
  	  	  	  	  	  	  	  	  	  });

void toggle_switch(char *buffer);
