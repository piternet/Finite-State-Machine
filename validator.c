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
	int stdin_copy = dup(0);
	int stdout_copy = dup(1);
	int pipe_dsc[2];
	if (pipe(pipe_dsc) == -1) {
		perror("pipe in validator\n");
	}
	char *w = malloc(sizeof(char) * MAXLEN);
	scanf(" %s", w);
	if (close(0) == -1)
		perror("close in validator\n");
	if (dup(pipe_dsc[0]) != 0)
		perror("dup in validator\n");
	if (close(1) == -1)
		perror("close in validator\n");
	if (dup(pipe_dsc[1]) != 1)
		perror("dup in validator\n");
	// if (close(pipe_dsc[0]) == -1)
	// 	perror("close in validator\n");
	// if (close(pipe_dsc[1]) == -1)
	// 	perror("close in validator\n");
	switch (fork()) {
		case -1:
			perror("fork in validator\n");
		case 0:
			execl("run", "run", (char *) NULL);
			perror("execlp in validator\n");
		default:		
			printMachine(m);
			printf("%s\n", w);
			if (wait(0) == -1)
				perror("wait in validator\n");
			dup2(stdin_copy, 0);
			dup2(stdout_copy, 1);
			close(stdin_copy);
			close(stdout_copy);
			char * result;
			scanf(" %s", result);
			printf("wynik = %s\n", result);
			
	}
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