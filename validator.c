#include "helper.h"

void printMachine(Machine *m) {
	printf("%d %d %d %d %d\n", m->N, m->A, m->Q, m->U, m->F);
	printf("%d\n", m->q);
	int i = 0;
	while(i < m->accSize) {
		printf("%d ", m->acc[i++]);
	}
	printf("\n");
	for(i=0; i<MAXSET; i++) {
		int j;
		for(j=0; j<ALPHSIZE; j++) {
			if(m->transSize[i][j] > 0) {
				printf("%d %c ", i, (char) (j+INTA));
				int k;
				for(k=0; k<m->transSize[i][j]; k++) {
					printf("%d ", m->trans[i][j][k]);
				}
				printf("\n");
			}
		}
	}
}


void waitForWords() {

}

void testRun(Machine *m) {
	// make pipe, dup, execve run
	int pipe_dsc[2];
	if (pipe(pipe_dsc) == -1) {
		perror("pipe in validator\n");
	}
	char *w = malloc(sizeof(char) * MAXLEN);
	scanf(" %s", w);
	switch (fork()) {
		case -1:
			perror("fork in validator\n");
		case 0:
			if (close(0) == -1)
				perror("close in validator\n");
			if (dup(pipe_dsc[0]) != 0)
				perror("dup in validator\n");
			char pipe_write_dsc_str[10];
			sprintf(pipe_write_dsc_str, "%d", pipe_dsc[1]);

			execl("run", "run", pipe_write_dsc_str, (char *) NULL);
			perror("execlp in validator\n");
		default: ;	
			int stdout_copy = dup(1);
			if (close(1) == -1)
				perror("close in validator\n");
			if (dup(pipe_dsc[1]) != 1)
				perror("dup in validator\n");
			
			printMachine(m);
			printf("%s\n", w);
			if (wait(0) == -1)
				perror("wait in validator\n");
			char msg[1];
			int len;
			if ((len = read(pipe_dsc[0], msg, sizeof(char)) != sizeof(char)) == -1)
				perror("read in validator\n");
			dup2(stdout_copy, 1);
			if (close(stdout_copy) == -1)
				perror("close in validator\n");
			if (close(pipe_dsc[0]) == -1)
				perror("close in validator\n");
			if (close(pipe_dsc[1]) == -1)
				perror("close in validator\n");
			printf("msg=%s\n", msg);
	}
}

void server(Machine *m, Validator *v) {
	/*
	*	1. Check if we got any new tester - if yes, add it.
		2. Check if any tester sent some word.
			2a. If he did - fork new run process and send him the word.
			2b. If the word sent == '!':
		3. Check if any run process did sent something back to us.
	/*
	// if (poll(&(struct pollfd){ .fd = fd, .events = POLLIN }, 1, 0)==1)
    /* data available */
    mqd_t testers = mq_open(MQ_NAME_TESTERS, O_RDWR | O_CREAT, 0777, NULL);
    if (testers == (mqd_t) -1) {
    	perror("mq_open in validator\n");
    }
    while (true) {
    	/* Check if we got any new tester */
    	int ret, buff_size;
    	char buff[MAXLEN];
    	ret = mq_receive(testers, buff, buff_size, NULL);
    	if (ret < 0) {
    		if (errno != EAGAIN)
    			perror("mq_receive in validator\n");
    	}
    	else {
    		/* We got new tester */
    		// v->testers[v->testersSize]
    		printf("new tester, his msg=%s\n", buff);
    		v->testersSize++;
    	}
    }
}

int main() {
	Machine *machine = malloc(sizeof(Machine));
	Validator *validator = malloc(sizeof(Validator));
	validator->testersSize = 0;
	readInput(machine);
	printMachine(machine);
	
	server(machine, validator);
	//testRun(machine);

	free(machine);
	return 0;
}