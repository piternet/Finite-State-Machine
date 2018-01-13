#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "helper.h"

void readInput() {
	char line[MAXLEN];
	while(fgets(line, MAXLEN, stdin) != NULL) {
		if(line[0] == ENDCHAR) {
			// zakonczenie pracy
		}
		else {
			// zwykle slowo do wyslania
		}
	}
}

int main() {
	readInput();
	return 0;
}