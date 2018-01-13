#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "helper.h"

void readInput(Machine *m) {
	/* assuming that input is correct */
	scanf("%d %d %d %d %d", &(m->N), &(m->A), &(m->Q), &(m->U), &(m->F));
	scanf("%d ", &(m->q));
	char line[MAXLINE], *chrValue;
	fgets(line, MAXLINE, stdin);
	chrValue = strtok(line, " ");
	int value, i = 0, vr = 0;
	vr = sscanf(chrValue, "%d", &value);
	while (vr > 0) {
		m->acc[i++] = value;
		chrValue = strtok(NULL, " ");
		vr = (chrValue == NULL) ? 0 : sscanf(chrValue, "%d", &value);
	}
	m->accSize = i;
	int lines = m->N - 3;
	while(lines-- > 0) {
		int q; char a;
		fgets(line, MAXLINE, stdin);
		chrValue = strtok(line, " ");
		vr = sscanf(chrValue, "%d ", &q);
		chrValue = strtok(NULL, " ");
		vr = sscanf(chrValue, "%c ", &a);
		chrValue = strtok(NULL, " ");
		vr = sscanf(chrValue, "%d", &value);
		i = 0;
		while (vr > 0) {
			m->trans[q][(int)a-'a'][i++] = value;
			chrValue = strtok(NULL, " ");
			vr = (chrValue == NULL) ? 0 : sscanf(chrValue, "%d", &value);
		}
		m->transSize[q][(int)a-'a'] = i;
	}
}

void printMachine(Machine *m) {
	printf("MASZYNA m:\n");
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
				printf("%d %c: ", i, (char) (j+'a'));
				int k;
				for(k=0; k<m->transSize[i][j]; k++) {
					printf("%d ", m->trans[i][j][k]);
				}
				printf("\n");
			}
		}
	}
}

int main() {
	Machine *machine = malloc(sizeof(Machine));
	readInput(machine);
	printMachine(machine);
	free(machine);
	return 0;
}