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

void server() {
	// if (poll(&(struct pollfd){ .fd = fd, .events = POLLIN }, 1, 0)==1)
    /* data available */
}

int main() {
	Machine *machine = malloc(sizeof(Machine));
	readInput(machine);
	printMachine(machine);
	/* open queue, both for reading and writing */
	/* mqd_t mq = mq_open(mq_name, O_RDWR | O_CREAT);
	waitForWords(); */

	testRun(machine);

	free(machine);
	return 0;
}