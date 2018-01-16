#include "helper.h"

bool accept(Machine *m, char *w, int *r, int ri, int val) {
	r[ri] = val;
	int i = 0;
	
	if (ri >= strlen(w)) {
		// check if last state is accepting
		while(i < m->accSize) {
			if (m->acc[i++] == r[ri])
				return true;
		}
		return false;
	}
	int wi = (int) w[ri] - INTA;
	bool result = r[ri] < m->U;
	pid_t child_pid, wpid;
	int pipe_dsc[2], status = 0;
	if (pipe(pipe_dsc) == -1) {
		// TODO: reagowanie na bledy systemowe
		perror("pipe in run\n");
	}
	// printf("pid=%d, ri=%d, r[ri]=%d; pipe_read=%d, pipe_write=%d\n", getpid(), ri, r[ri], pipe_dsc[0], pipe_dsc[1]);
	if (m->transSize[r[ri]][wi] > 1) {
		while (i < m->transSize[r[ri]][wi]) {
			switch (child_pid = fork()) {
				case -1:
					// TODO: reagowanie na bledy systemowe
					perror("fork in run\n");
				case 0: ;
					// child
					if (close(pipe_dsc[0]) == -1) {
						// TODO: reagowanie na bledy systemowe
						perror("close in run\n");
					}
					bool my_result = accept(m, w, r, ri+1, m->trans[r[ri]][wi][i]);
					char msg[1];
					msg[0] = my_result ? '1' : '0';
					if (write(pipe_dsc[1], msg, sizeof(msg)) != sizeof(msg)) {
						// TODO: reagowanie na bledy systemowe
						perror("write in run\n");
					}
					// printf("pid=%d, write to fd=%d done, result = '%s'\n", getpid(), pipe_dsc[1], msg);
					exit(0);
				default:
					i++;
			}
		}

		while ((wpid = wait(&status)) > 0); // wait for all children
		if (close(pipe_dsc[1]) == -1) {
			// TODO: reagowanie na bledy systemowe
			perror("close in run\n");
		}
		i = 0;
		while (i < m->transSize[r[ri]][wi]) {
			char msg[MAXLEN];
			int len;
			if ((len = read(pipe_dsc[0], msg, sizeof(char))) == -1) {
				// TODO: reagowanie na bledy systemowe
				perror("read in run\n");
			}
			msg[len] = '\0';
			bool my_result = (strcmp(msg, "1") == 0);
			// printf("pid=%d, read from fd=%d done, result = '%s'\n", getpid(), pipe_dsc[0], msg);
			if (r[ri] < m->U)
				result &= my_result;
			else
				result |= my_result;
			i++;	
		}
		return result;

	}
	else {
		return accept(m, w, r, ri+1, m->trans[r[ri]][wi][i]);;
	}
}

int main(int argc, char *argv[]) {
	Machine *m = malloc(sizeof(Machine));
	readInput(m);
	char *w = malloc(sizeof(char) * MAXLEN);
	int *r = malloc(sizeof(int) * MAXLEN);
	int write_dsc = atoi(argv[1]);
	scanf(" %s", w);
	if (accept(m, w, r, 0, m->q))
		if (write(write_dsc, "1", sizeof(char)) != sizeof(char))
			perror("write in run\n");
	else
		if (write(write_dsc, "0", sizeof(char)) != sizeof(char))
			perror("write in run\n");
	free(w);
	free(r);
	free(m);
	return 0;
}