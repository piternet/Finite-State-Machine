#ifndef HELPER_H_
#define HELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

#include <sys/wait.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <mqueue.h>
#include <sys/wait.h>

#define MAXLEN 			1000 + 3
#define MAXLINE 		1000000
#define MAXSET 			100 + 3
#define MAXPROC			49152
#define ALPHSIZE 		26
#define ENDCHAR			'!'
#define INTA			(int)('a')
#define MQ_NAME_TESTERS	"/testers"
#define MSGSIZE			8192
#define EMPTYCHAR		'.'
#define EMPTYCHARSTR	"."
#define MQ_MODE			0666
#define DEBUG 			1

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

typedef struct Tester {
	pid_t pid;
	int snt, rcd, acc;
	mqd_t mq_write, mq_read;
	int pipe_dsc[2];
	bool dead;
} Tester;

typedef struct Validator {
	int rcd, snt, acc;
	Tester testers[MAXPROC];
	int testersSize;
	int pipe_snt[2];
	struct mq_attr attr;
} Validator;

//char *mq_name = "/machineQueue";

void readInput(Machine *m);

#endif // HELPER_H_