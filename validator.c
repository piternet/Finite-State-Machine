#include "helper.h"

void closeTester(Validator *v, int i) {
	if (mq_close(v->testers[i].mq_read))
		perror("mq_close testers in validator\n");
	if (mq_close(v->testers[i].mq_write))
		perror("mq_close testers in validator\n");
	if (close(v->testers[i].pipe_dsc[0]))
		perror("close pipe in validator\n");
	if (close(v->testers[i].pipe_dsc[1]))
		perror("close pipe in validator\n");
	// testers should do mq_unlink
}

void terminate(Machine *m, Validator *v, mqd_t testers) {
	if (mq_close(testers))
		perror("mq_close testers in validator\n");
	if (mq_unlink(MQ_NAME_TESTERS))
		perror("mq_unlink testers in validator\n");
	for (int i=0; i<v->testersSize; i++) {
		if (!v->testers[i].dead)
			closeTester(v, i);
	}
	if (close(v->pipe_snt[0]))
		perror("close pipe in validator\n");
	if (close(v->pipe_snt[1]))
		perror("close pipe in validator\n");
	free(m);
	free(v);
	exit(0);
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
	int buff_len = v->testersSize+10;
	char *buffer = malloc(buff_len * sizeof(char));
	for (int i=0; i<v->testersSize; i++) {
		bzero(buffer, buff_len);
		int len, res = 0;
		if ((len = read(v->testers[i].pipe_dsc[0], buffer, buff_len)) == -1)
    		perror("read in validator\n");
    	for (int j=0; j<strlen(buffer); j++)
    		if (buffer[j] == '1')
    			res++;
    	v->acc += res;
    	v->testers[i].acc = res;
	}
	bzero(buffer, buff_len);
	if ((buff_len = read(v->pipe_snt[0], buffer, buff_len)) == -1)
    	perror("read in validator\n");
    for (int j=0; j<strlen(buffer); j++)
		if (buffer[j] == '1')
			v->snt++;
	printf("Rcd: %d\n", v->rcd);
	printf("Snt: %d\n", v->snt);
	printf("Acc: %d\n", v->acc);
	for (int i=0; i<v->testersSize; i++) {
		printf("PID: %d\n", (int) v->testers[i].pid);
		printf("Rcd: %d\n", v->testers[i].rcd);
		printf("Acc: %d\n", v->testers[i].acc);
	}
	free(buffer);
}

bool testRun(Machine *m, char *w) {
	// make pipe, dup, execve run
	//printf("x\n");
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
			int status = 0;
			if (wait(&status) == -1)
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
			return msg[0] == '1' ? true : false;
	}
	return false;
}

void sendKillSignals(Validator *v) {
	// send kill signals to all testers
	for (int i=0; i<v->testersSize; i++) {
		if(DEBUG) printf("killing %d\n", v->testers[i].pid);
		if (v->testers[i].dead)
			continue;
		kill(v->testers[i].pid, SIGRTMIN);
	}
}

void newTester(Machine *m, Validator *v, char *buff) {
	if(buff[0] == ENDCHAR) {
		// dead message
		pid_t tester_pid = (pid_t) atoi(buff+1);
		for (int i=0; i<v->testersSize; i++) {
			if (v->testers[i].pid == tester_pid) {
				v->testers[i].dead = true;
				closeTester(v, i);
			}
		}
		return;
	}
	pid_t tester_pid = (pid_t) atoi(buff);
	v->testers[v->testersSize].pid = tester_pid;
	v->testers[v->testersSize].rcd = 0;
	v->testers[v->testersSize].acc = 0;
	v->testers[v->testersSize].dead = false;
	if(DEBUG) printf("new tester, pid=%d\n", v->testers[v->testersSize].pid);
	char name_write[10], name_read[10];
	char pid_str[6];
	sprintf(pid_str, "%d", (int) tester_pid);
    strcpy(name_write, "/w");
    strcat(name_write, pid_str);
    strcpy(name_read, "/r");
    strcat(name_read, pid_str);
    if(DEBUG) printf("%s %s\n", name_write, name_read);
    v->testers[v->testersSize].mq_write = mq_open(name_write, O_RDWR | O_NONBLOCK | O_CREAT, MQ_MODE, &v->attr);
    v->testers[v->testersSize].mq_read = mq_open(name_read, O_RDWR | O_CREAT, MQ_MODE, &v->attr);
    if (pipe(v->testers[v->testersSize].pipe_dsc) == -1) {
		perror("pipe in validator\n");
	}
	if (write(v->testers[v->testersSize].pipe_dsc[1], "0", sizeof(char)) != sizeof(char)) {
		perror("write in validator\n");
	}
	v->testersSize++;
	free(buff);
}

void handleTester(Machine *m, Validator *v, int i, mqd_t mq_read, char *buff) {
	if(DEBUG) printf("tester with pid %d sent '%s'\n", v->testers[i].pid, buff);
	char *w = malloc(sizeof(char) * MAXLEN);
	if (strlen(buff) == 0)
		strcpy(buff, EMPTYCHARSTR);
	strcpy(w, buff);
	bool run = testRun(m, w);
	if(DEBUG) printf("mam odp od pid %d\n ", v->testers[i].pid);
	if (run) {
		if (write(v->testers[i].pipe_dsc[1], "1", sizeof(char)) != sizeof(char)) {
			perror("write in handleTester\n");
		}
	}
	char answer[2];
	answer[0] = run ? 'A' : 'N';
	answer[1] = '\0';
	char send_buff[MAXLEN+5];
	if(strcmp(w, EMPTYCHARSTR) != 0)
		strcpy(send_buff, w);
	else
		strcpy(send_buff, "");
	strcat(send_buff, " ");
	strcat(send_buff, answer);
	int ret = mq_send(mq_read, send_buff, strlen(send_buff), 1);
	if (ret < 0) {
		perror("mq_send in validator\n");
	}
	if (write(v->pipe_snt[1], "1", sizeof(char)) != sizeof(char)) {
		perror("write in handleTester\n");
	}
	if(DEBUG) printf("sent %s to %d\n", send_buff, v->testers[i].pid);
	free(buff);
	free(w);
}

void server(Machine *m, Validator *v) {
	/*
		1. Check if we got any new tester - if yes, add it.
		2. Check if any tester sent some word.
			2a. If he did - fork new run process and send him the word.
			2b. If the word sent == '!':
		3. Check if any run process did sent something back to us.
	*/
    mqd_t testers = mq_open(MQ_NAME_TESTERS, O_RDWR | O_NONBLOCK | O_CREAT, MQ_MODE, &v->attr);
    if (testers == (mqd_t) -1) {
    	perror("mq_open in validator\n");
    }
    while (true) {
    	/* Check if we got any new tester */
    	int ret, buff_size;
    	char buff[MSGSIZE];
    	ret = mq_receive(testers, buff, MSGSIZE, NULL);
    	if (ret < 0) {
    		if (errno != EAGAIN)
    			perror("mq_receive testers mq in validator\n");
    	}
    	else {
    		/* We got new tester */
    		char *tester_buff = malloc(strlen(buff) * sizeof(char));
    		strcpy(tester_buff, buff);
    		newTester(m, v, tester_buff);
    	}
    	/* Check if any tester sent smth */
    	for (int i=0; i<v->testersSize; i++) {
    		if (v->testers[i].dead)
    			continue;
    		bzero(buff, MSGSIZE);
    		mqd_t mq_write = v->testers[i].mq_write, mq_read = v->testers[i].mq_read;
    		ret = mq_receive(mq_write, buff, MSGSIZE, NULL);
	    	if (ret < 0) {
	    		if (errno != EAGAIN)
	    			perror("mq_receive from one tester in validator\n");
	    	}
	    	else {
	    		/* terminate */
	    		if (buff[0] == ENDCHAR) {
	    			/* shut validator down */
	    			int status = 0; pid_t wpid;
	    			while ((wpid = wait(&status)) > 0); // wait for all children
	    			/* send all testers signals to shut down */
	    			sendKillSignals(v);
	    			printOutput(v);
	    			terminate(m, v, testers);
	    		}
	    		else {
	    			/* handle tester's word */
	    			v->rcd++;
					v->testers[i].rcd++;
	    			switch(fork()) {
		    			case -1:
		    				perror("fork in validator\n");
		    			case 0: ;
		    				if (close(v->testers[i].pipe_dsc[0]) != 0)
		    					perror("close in validator\n");
		    				if (close(v->pipe_snt[0]) != 0)
		    					perror("close in validator\n");
		    				char *tester_buff = malloc(strlen(buff) * sizeof(char));
		    				strcpy(tester_buff, buff);
		    				handleTester(m, v, i, mq_read, tester_buff);
		    				if(DEBUG) printf("ending fork...\n");
		    				exit(0);
		    			default:
		    				continue;
	    			}
	    		}
	    		
	    	}
    	}
    }
}

int main() {
	setbuf(stdout, NULL);
	Machine *machine = malloc(sizeof(Machine));
	Validator *validator = malloc(sizeof(Validator));
	validator->testersSize = 0;
	if (pipe(validator->pipe_snt) == -1) {
		perror("pipe in validator\n");
	}
	if (write(validator->pipe_snt[1], "0", sizeof(char)) != sizeof(char)) {
		perror("write in validator\n");
	}
	validator->attr.mq_maxmsg = 10;
	validator->attr.mq_msgsize = 8192;
	readInput(machine);
	if(DEBUG) printMachine(machine);
	server(machine, validator);
	return 0;
}