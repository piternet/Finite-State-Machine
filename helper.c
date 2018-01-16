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
			m->trans[q][(int)a-INTA][i++] = value;
			chrValue = strtok(NULL, " ");
			vr = (chrValue == NULL) ? 0 : sscanf(chrValue, "%d", &value);
		}
		m->transSize[q][(int)a-INTA] = i;
	}
}