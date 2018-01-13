#define MAXLEN 		1000 + 3
#define MAXLINE 	1000000
#define MAXSET 		100 + 3
#define ALPHSIZE 	26
#define ENDCHAR		'!'

typedef struct Machine {
	/* machine consts */
	int N, A, Q, U, F;
	/* beginning state */
	int q;
	/* accepting states */
	int accSize, acc[MAXSET];
	/* transitions between states */
	int transSize[MAXSET][ALPHSIZE], trans[MAXSET][ALPHSIZE][MAXSET];
} Machine;
