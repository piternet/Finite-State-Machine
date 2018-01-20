#include "helper.h"

void readAndSend() {
	char pid_str[6];
	sprintf(pid_str, "%d", (int) getpid());
	printf("mypid=%s\n", pid_str);
	char line[MAXLEN];
	mqd_t testers = mq_open(MQ_NAME_TESTERS, O_RDWR | O_CREAT, 0777, NULL);
    if (testers == (mqd_t) -1) {
    	perror("mq_open in tester\n");
    }
    int ret = mq_send(testers, pid_str, strlen(pid_str), 1);
    if (ret < 0) {
    	perror("mq_send in tester\n");
    }
	while(fgets(line, MAXLEN, stdin) != NULL) {
		if(line[0] == ENDCHAR) {
			// zakonczenie pracy
		}
		else {
			// zwykle slowo do wyslania
			printf("%s\n", line);
		}
	}
}

int main() {
	readAndSend();
	return 0;
}