#include "helper.h"

typedef struct {
	mqd_t testers, read, write;
	char name_read[10], name_write[10];
} Mqs;

Tester *t;
Mqs mqs;

void simpleterminate() {
	free(t);
	exit(0);
}

void terminate() {
	if (mq_close(mqs.testers))
		perror("mq_close testers in validator\n");
	if (mq_close(mqs.read))
		perror("mq_close read in validator\n");
	if (mq_close(mqs.write))
		perror("mq_close write in validator\n");
	if (mq_unlink(mqs.name_read))
		perror("mq_unlink read in validator\n");
	if (mq_unlink(mqs.name_write))
		perror("mq_unlink write in validator\n");
	free(t);
	exit(0);
}

void printOutput() {
	printf("Snt: %d\n", t->snt);
	printf("Rcd: %d\n", t->rcd);
	printf("Acc: %d\n", t->acc);
}

void receiveMessages() {
	char buff[MSGSIZE];
	bzero(buff, MSGSIZE);
	int ret = mq_receive(mqs.read, buff, MSGSIZE, NULL);
	if (ret < 0) {
		if (errno != EAGAIN) {
			perror("mq_receive in tester\n");
			terminate();
		}
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

void sendDeadMessage() {
	char pid_str[7];
	pid_str[0] = ENDCHAR;
	sprintf(pid_str+1, "%d", (int) getpid());
	int ret = mq_send(mqs.testers, pid_str, strlen(pid_str), 1);
    if (ret < 0) {
    	perror("mq_send in tester\n");
    	simpleterminate();
    }
}

void catch(int sig) {
	while (t->snt > t->rcd) {
		receiveMessages();
	}
	printOutput();
	terminate();
	exit(0);
}


void readAndSend() {
	struct mq_attr attr;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = 8192;
	t->pid = getpid();
	char pid_str[6];
	sprintf(pid_str, "%d", (int) getpid());
	printf("PID: %s\n", pid_str);
	char line[MAXLEN];
	mqs.testers = mq_open(MQ_NAME_TESTERS, O_WRONLY | O_NONBLOCK | O_CREAT, MQ_MODE, &attr);
    if (mqs.testers == (mqd_t) -1) {
    	perror("mq_open in tester\n");
    	simpleterminate();
    }
    int ret = mq_send(mqs.testers, pid_str, strlen(pid_str), 1);
    if (ret < 0) {
    	perror("mq_send in tester\n");
    	simpleterminate();
    }
    strcpy(mqs.name_write, "/w");
    strcat(mqs.name_write, pid_str);
    strcpy(mqs.name_read, "/r");
    strcat(mqs.name_read, pid_str);
    //printf("%s %s\n", name_write, name_read);
    mqs.write = mq_open(mqs.name_write, O_WRONLY | O_NONBLOCK | O_CREAT, MQ_MODE, &attr);
    mqs.read = mq_open(mqs.name_read, O_RDONLY | O_CREAT | O_NONBLOCK, MQ_MODE, &attr);
    if (mqs.write == (mqd_t) -1 || mqs.read == (mqd_t) -1) {
    	perror("mq_open in tester\n");
    	terminate();
    }
    char buff[MSGSIZE];
	while(true) {
		if (fgets(line, MAXLEN, stdin) != NULL) {
			line[strlen(line)-1] = '\0'; // cut last character == '\n'
			ret = mq_send(mqs.write, line, strlen(line), 1);
			//printf("sent '%s' to validator\n", line);
		    if (ret < 0) {
		    	perror("mq_send in tester\n");
		    	terminate();
		    }
		    if (line[0] != ENDCHAR)
		   		t->snt++;
		}
		else {
			if (t->rcd == t->snt) {
				sendDeadMessage();
				printOutput();
				terminate();
			}
		}
	    receiveMessages();   
	}
}

void handleSignal() {
	struct sigaction action;
	sigset_t block_mask;
	sigemptyset(&block_mask);
	action.sa_handler = catch;
	action.sa_mask = block_mask;
	action.sa_flags = 0;
	if (sigaction(SIGRTMIN, &action, 0) == -1)
		perror("sigaction in tester\n");  
	if (sigprocmask(SIG_BLOCK, &block_mask, 0) == -1)
		perror("sigprocmask in tester\n");
}

int main() {
	t = malloc(sizeof(Tester));
	t->rcd = 0;
	t->acc = 0;
	t->snt = 0;
	handleSignal();
	readAndSend();
	printOutput();
	return 0;
}