#include <stdio.h>
#include <stdlib.h>
#include "hmm.h"

#define MAX_MODELS 10

void usage(){
	printf("[Usage]:  ./test modellist.txt testing_datax.txt result.txt\n");
	exit(-1);
}

int main(int argc, char* argv[]){
	if (argc != 4) usage();
	//printf("ok\n");
	char model_list_filename[MAX_FILENAME_LENGTH] = "";
	char testing_data_filename[MAX_FILENAME_LENGTH] = "";
	char result_filename[MAX_FILENAME_LENGTH] = "";
	char seq[MAX_TOTAL_SEQ][MAX_SEQ_LEN];
	int totalSeq = 0;
	int testfile_id = strcmp(argv[2], "testing_data1.txt") == 0? 1:strcmp(argv[2], "testing_data2.txt") == 0? 2:0;
	strcat(model_list_filename, argv[1]);
	strcat(testing_data_filename, argv[2]);
	strcat(result_filename, argv[3]);
	loadSeq(seq, testing_data_filename);
	FILE* resultFile = fopen(result_filename, "w");
	for(int i = 0; seq[i][0] != '\0' && i < MAX_TOTAL_SEQ; i++) totalSeq++;
	HMM models[MAX_MODELS];
	int result[MAX_TOTAL_SEQ];
	int models_count = load_models(model_list_filename, models, MAX_MODELS);
	//dump_models(models, models_count);
	printf("totalSeq: %d, models_count: %d\n", totalSeq, models_count);
	for(int i = 0; i < totalSeq; i++){
		double max_prob = 0.0;
		int max_model = 0;
		for(int m = 0; m < models_count; m++){
			//printf("%s\n", seq[i]);
			double viterbi_prob = viterbi(&models[m], seq[i]);
			if (viterbi_prob > max_prob) max_prob = viterbi_prob, max_model = m;
		}
		result[i] = max_model;
		fprintf(resultFile, "model_0%d.txt %.7e\n", max_model+1, max_prob);
	}
	if (testfile_id == 1){
		FILE* answerFile = open_or_die("testing_answer.txt", "r");
		FILE* accFile = fopen("acc.txt", "w");
		int correct = 0;
		for(int i = 0; i < totalSeq; i++){
			char ans_str[64];
			fscanf(answerFile, "%s", ans_str);
			//printf("%s\n", &models[result[i]]->model_name);
			if(strcmp(ans_str, models[result[i]].model_name) == 0) correct++;
			//printf("%d\n", correct);
			//printf("%s\n", ans_str);
		}
		printf("accuracy: %.6lf\n", (double)correct/totalSeq);
		fprintf(accFile, "%.6lf", (double)correct/totalSeq);
	}
	return 0;
}
// ./train 20 model_init.txt seq_model_01.txt model_01.txt
// ./train 20 model_init.txt seq_model_02.txt model_02.txt
// ./train 20 model_init.txt seq_model_03.txt model_03.txt
// ./train 20 model_init.txt seq_model_04.txt model_04.txt
// ./train 20 model_init.txt seq_model_05.txt model_05.txt