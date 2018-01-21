#include "helper.h"

void kill(Tester *t) {
	free(t);
}

void printOutput(Tester *t) {
	printf("Snt: %d\n", t->snt);
	printf("Rcd: %d\n", t->rcd);
	printf("Acc: %d\n", t->acc);
}

void readAndSend(Tester *t) {
	t->pid = getpid();
	char pid_str[6];
	sprintf(pid_str, "%d", (int) getpid());
	printf("PID: %s\n", pid_str);
	char line[MAXLEN];
	mqd_t testers = mq_open(MQ_NAME_TESTERS, O_WRONLY | O_NONBLOCK | O_CREAT, MQ_MODE, NULL);
    if (testers == (mqd_t) -1) {
    	perror("mq_open in tester\n");
    }
    int ret = mq_send(testers, pid_str, strlen(pid_str), 1);
    if (ret < 0) {
    	perror("mq_send in tester\n");
    }
    char name_write[10], name_read[10];
    strcpy(name_write, "/w");
    strcat(name_write, pid_str);
    strcpy(name_read, "/r");
    strcat(name_read, pid_str);
    printf("%s %s\n", name_write, name_read);
    mqd_t mq_write = mq_open(name_write, O_RDWR | O_NONBLOCK | O_CREAT, MQ_MODE, NULL);
    mqd_t mq_read = mq_open(name_read, O_RDWR | O_CREAT | O_NONBLOCK, MQ_MODE, NULL);
    if (mq_write == (mqd_t) -1 || mq_read == (mqd_t) -1) {
    	perror("mq_open in tester\n");
    }
    char buff[MSGSIZE];
	while(true) {
		if (fgets(line, MAXLEN, stdin) != NULL) {
			line[strlen(line)-1] = '\0'; // cut last character == '\n'
			ret = mq_send(mq_write, line, strlen(line), 1);
			printf("sent '%s' to validator\n", line);
		    if (ret < 0) {
		    	perror("mq_send in tester\n");
		    }
		    t->snt++;
		}
	    bzero(buff, MSGSIZE);
	    int buff_size;
	    ret = mq_receive(mq_read, buff, MSGSIZE, NULL);
	    if (ret < 0) {
    		if (errno != EAGAIN)
    			perror("mq_receive in tester\n");
    	}
    	else {
    		// we got some word from validator
    		t->rcd++;
    		printf("%s\n", buff);
    		char *w = strtok(buff, " ");
    		char *w2 = strtok(NULL, " ");
    		if (w2 == NULL) {
    			if (w[0] == 'A')
    				t->acc++;
    		}
    		else if (w2[0] == 'A')
	    		t->acc++;
    	}	    
	}
	// TODO:close + unlink mq
}

int main() {
	Tester *tester = malloc(sizeof(Tester));
	tester->rcd = 0;
	tester->acc = 0;
	tester->snt = 0;
	readAndSend(tester);
	printOutput(tester);
	kill(tester);
	return 0;
}