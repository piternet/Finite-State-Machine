#include "helper.h"

void kill(Machine *m, Validator *v) {
	free(m);
	free(v);
}

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


void printOutput(Validator *v) {
	printf("Rcd: %d\n", v->rcd);
	printf("Snt: %d\n", v->snt);
	printf("Acc: %d\n", v->acc);
	for (int i=0; i<v->testersSize; i++) {
		printf("PID: %d\n", (int) v->testers[i].pid);
		printf("Rcd: %d\n", v->testers[i].rcd);
		printf("Acc: %d\n", v->testers[i].acc);
	}
}

bool testRun(Machine *m, char *w) {
	// make pipe, dup, execve run
	printf("test_run, w = %s\n", w);
	int pipe_dsc[2];
	if (pipe(pipe_dsc) == -1) {
		perror("pipe in validator\n");
	}
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
			printf("odpalam process run\n");
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
			printf("czekam na odp od run...\n");
			if ((len = read(pipe_dsc[0], msg, sizeof(char)) != sizeof(char)) == -1)
				perror("read in validator\n");
			printf("mam odp od run...\n");
			dup2(stdout_copy, 1);
			if (close(stdout_copy) == -1)
				perror("close in validator\n");
			if (close(pipe_dsc[0]) == -1)
				perror("close in validator\n");
			if (close(pipe_dsc[1]) == -1)
				perror("close in validator\n");
			return msg[0] == '1' ? true : false;
	}
	return false;
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
    mqd_t testers = mq_open(MQ_NAME_TESTERS, O_RDONLY | O_NONBLOCK | O_CREAT, 0777, NULL);
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
    		pid_t tester_pid = (pid_t) atoi(buff);
    		v->testers[v->testersSize].pid = tester_pid;
    		v->testers[v->testersSize].rcd = 0;
    		v->testers[v->testersSize].acc = 0;
    		printf("new tester, pid=%d\n", v->testers[v->testersSize].pid);
    		char name_write[10], name_read[10];
    		char pid_str[6];
			sprintf(pid_str, "%d", (int) tester_pid);
		    strcpy(name_write, "/w");
		    strcat(name_write, pid_str);
		    strcpy(name_read, "/r");
		    strcat(name_read, pid_str);
		    printf("%s %s\n", name_write, name_read);
		    printf("testsize=%d\n", v->testersSize);
		    v->testers[v->testersSize].mq_write = mq_open(name_write, O_RDWR | O_NONBLOCK | O_CREAT, 0777, NULL);
		    v->testers[v->testersSize].mq_read = mq_open(name_read, O_RDWR | O_CREAT, 0777, NULL);
    		v->testersSize++;
    	}
    	/* Check if any tester sent smth */
    	for (int i=0; i<v->testersSize; i++) {
    		bzero(buff, MAXLEN);
    		mqd_t mq_write = v->testers[i].mq_write, mq_read = v->testers[i].mq_read;
    		ret = mq_receive(mq_write, buff, buff_size, NULL);
	    	if (ret < 0) {
	    		if (errno != EAGAIN)
	    			perror("mq_receive in validator\n");
	    	}
	    	else {
	    		/* Tester sent us smth */
	    		printf("tester with pid %d sent %s\n", v->testers[i].pid, buff);
	    		char *w = malloc(sizeof(char) * MAXLEN);
	    		strcpy(w, buff);
	    		bool run = testRun(m, w);
	    		printf("ok, tested it on run\n");
	    		char answer[2];
	    		answer[0] = run ? 'A' : 'N';
	    		ret = mq_send(mq_read, answer, strlen(answer), 1);
	    		if (ret < 0) {
	    			perror("mq_send in validator\n");
	    		}
	    		printf("sent %c to %d\n", answer[0], v->testers[i].pid);
	    	}
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

	kill(machine, validator);
	return 0;
}