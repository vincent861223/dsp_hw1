#include <stdio.h>
#include <stdlib.h>
#include "hmm.h"

#define BATCH_SIZE 500

void usage(){
	printf("[Usage]:  ./train $iter model_init.txt seq_model_xx.txt model_xx.txt\n");
	exit(-1);
}

int main(int argc, char** argv){
	if (argc != 5) usage();
	int iter = atoi(argv[1]);
	char model_init_filename[MAX_FILENAME_LENGTH] = "";
	char seq_model_filename[MAX_FILENAME_LENGTH] = "";
	char output_model_filename[MAX_FILENAME_LENGTH] = "";
	char seq[MAX_TOTAL_SEQ][MAX_SEQ_LEN];
	int totalSeq = 0;
	strcat(model_init_filename, argv[2]);
	strcat(seq_model_filename, argv[3]);
	strcat(output_model_filename, argv[4]);
	HMM model;
	loadHMM(&model, model_init_filename);
	//dumpHMM(stderr, &model);
	loadSeq(seq, seq_model_filename);
	for(int i = 0; seq[i][0] != '\0' && i < MAX_TOTAL_SEQ; i++) totalSeq++;
	#ifdef DEBUG
	printf("totalSeqences: %d\n", totalSeq);
	#endif
	while(iter--){
		printf("\n\n==============================iterations left = %d================================\n", iter);
		dumpHMM(stdout, &model);
		double total = 0.0;
		for(int n = 0; n < totalSeq; n++) total += viterbi(&model, seq[n]);
		printf("prob: %.100lf\n", total);
		train(&model, seq, BATCH_SIZE, totalSeq);
	}
	FILE* ouputFile = fopen(output_model_filename, "w");
	dumpHMM(ouputFile, &model);
	//dumpSeq(seq);
	//printf("seq: %s\n", seq[0]);
	//printf("test\n");
	return 0;
}
