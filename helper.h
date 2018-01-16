#ifndef HELPER_H_
#define HELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <mqueue.h>
#include <sys/wait.h>

#define MAXLEN 		1000 + 3
#define MAXLINE 	1000000
#define MAXSET 		100 + 3
#define ALPHSIZE 	26
#define ENDCHAR		'!'
#define INTA		(int)('a')

typedef struct Machine {
	/* machine consts */
	int N, A, Q, U, F;
	/* beginning state */
	int q;
	/* accepting states */
	int accSize, acc[MAXSET];
	/* transitions between states */
	int transSize[MAXSET][ALPHSIZE], trans[MAXSET][ALPHSIZE][MAXSET];
} Machine;

//char *mq_name = "/machineQueue";

void readInput(Machine *m);

#endif // HELPER_H_