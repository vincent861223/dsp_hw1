#include <stdio.h>
#include <stdlib.h>
#include "hmm.h"

void usage(){
	printf("[Usage]:  ./train $iter model_init.txt seq_model_xx.txt model_xx.txt\n");
	exit(-1);
}

int main(int argc, char** argv){
	if (argc != 5) usage();
	//FILE* fp = open_or_die
	int iter = atoi(argv[1]);
	char model_init_filename[MAX_FILENAME_LENGTH] = "../";
	char seq_model_filename[MAX_FILENAME_LENGTH] = "../";
	char output_model_filename[MAX_FILENAME_LENGTH] = "../";
	char seq[MAX_LINE][MAX_SEQ];
	int best_state_seq[MAX_SEQ];
	double alpha[MAX_SEQ][MAX_STATE];
	double beta[MAX_SEQ][MAX_STATE];
	double garma[MAX_SEQ][MAX_STATE];
	double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE];
	strcat(model_init_filename, argv[2]);
	strcat(seq_model_filename, argv[3]);
	strcat(output_model_filename, argv[4]);
	//printf("iter: %d\nmodel_init_filename: %s\nseq_model_filename: %s\noutput_model_filename: %s\n", iter, model_init_filename, seq_model_filename, output_model_filename);
	
	HMM model;
	loadHMM(&model, model_init_filename);
	//dumpHMM(stderr, &model);
	loadSeq(seq, seq_model_filename);

	// calc_alpha(alpha, &model, seq[0]);
	// calc_beta(beta, &model, seq[0]);
	// calc_garma(garma, alpha, beta, &model, seq[0]);
	// calc_epsilon(epsilon, alpha, beta, &model, seq[0]);
	// viterbi(best_state_seq, &model, seq[0]);
	// update_b(best_state_seq, &model, seq[0]);
	//dumpHMM(stderr, &model);
	while(iter--){
		for(int i = 0; seq[i][0] != '\0'; i++){
			printf("\n\n==============================seq = %d================================\n", i);
			dumpHMM(stdout, &model);
			calc_alpha(alpha, &model, seq[i]);
			calc_beta(beta, &model, seq[i]);
			calc_garma(garma, alpha, beta, &model, seq[i]);
			calc_epsilon(epsilon, alpha, beta, &model, seq[i]);
			viterbi(best_state_seq, &model, seq[i]);
			update_pi(garma, &model);
			update_a(epsilon, garma, &model, seq[i]);
			update_b(best_state_seq, garma, &model, seq[i]);
		}
	}
	
	//dumpSeq(seq);

	printf("seq: %s\n", seq[0]);
	//printf("test\n");
	return 0;
}