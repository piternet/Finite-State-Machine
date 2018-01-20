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
	mqd_t testers = mq_open(MQ_NAME_TESTERS, O_WRONLY | O_NONBLOCK | O_CREAT, 0777, NULL);
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
    mqd_t mq_write = mq_open(name_write, O_RDWR | O_NONBLOCK | O_CREAT, 0777, NULL);
    mqd_t mq_read = mq_open(name_read, O_RDWR | O_CREAT, 0777, NULL);
    if (mq_write == (mqd_t) -1 || mq_read == (mqd_t) -1) {
    	perror("mq_open in tester\n");
    }
	while(fgets(line, MAXLEN, stdin) != NULL) {
		line[strlen(line)-1] = '\0'; // cut last character == '\n'
		if(line[0] == ENDCHAR) {
			// zakonczenie pracy
		}
		else {
			// send normal word
			ret = mq_send(mq_write, line, strlen(line), 1);
			t->snt++;
			printf("sent '%s' to validator\n", line);
		    if (ret < 0) {
		    	perror("mq_send in tester\n");
		    }
		    char buff[2];
		    // TODO: puste slowo1!!!
		    int buff_size;
		    ret = mq_receive(mq_read, buff, buff_size, NULL);
		    if (ret < 0) {
		    	perror("mq_receive in tester\n");
		    }
		    printf("%s %c\n", line, buff[0]);
		    t->rcd++;
		    if (buff[0] == 'A')
		    	t->acc++;
		}
	}
	// TODO:close + unlink mq
}

int main() {
	Tester *tester = malloc(sizeof(Tester));
	t->rcd = 0;
	t->acc = 0;
	t->snt = 0;
	readAndSend(tester);
	printOutput(t);
	kill(tester);
	return 0;
}