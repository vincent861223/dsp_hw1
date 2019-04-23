#ifndef HMM_HEADER_
#define HMM_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#ifndef MAX_STATE
#   define MAX_STATE    10
#endif

#ifndef MAX_OBSERV
#   define MAX_OBSERV   26
#endif

#ifndef MAX_SEQ_LEN
#   define  MAX_SEQ_LEN     400
#endif

#ifndef MAX_TOTAL_SEQ
#   define MAX_TOTAL_SEQ 20000
#endif

#ifndef MAX_FILENAME_LENGTH
#   define MAX_FILENAME_LENGTH 100
#endif

typedef struct{
   char *model_name;                //model name
   int state_num;                   //number of state
   int observ_num;                  //number of b
   double pi[MAX_STATE];            //pi prob.
   double a[MAX_STATE][MAX_STATE];  //a prob.
   double b[MAX_STATE][MAX_OBSERV]; //b prob.
} HMM;

double alpha[MAX_TOTAL_SEQ][MAX_SEQ_LEN][MAX_STATE];
double beta[MAX_TOTAL_SEQ][MAX_SEQ_LEN][MAX_STATE];
double garma[MAX_TOTAL_SEQ][MAX_SEQ_LEN][MAX_STATE];
double epsilon[MAX_TOTAL_SEQ][MAX_SEQ_LEN][MAX_STATE][MAX_STATE];


static FILE *open_or_die( const char *filename, const char *ht )
{
   FILE *fp = fopen( filename, ht );
   if( fp == NULL ){
      char path[MAX_FILENAME_LENGTH] = "../";
      strcat(path, filename);
      fp = fopen( path, ht );
      if( fp == NULL){
        perror(filename);
        exit(1);
      }
   }

   return fp;
}

static void loadHMM( HMM *hmm, const char *filename )
{
   int i, j;
   FILE *fp = open_or_die( filename, "r");
   hmm->model_name = (char *)malloc( sizeof(char) * (strlen( filename)+10));
   strcpy( hmm->model_name, filename );

   char token[MAX_TOTAL_SEQ] = "";
   while( fscanf( fp, "%s", token ) > 0 )
   {
      if( token[0] == '\0' || token[0] == '\n' ) continue;

      if( strcmp( token, "initial:" ) == 0 ){
         fscanf(fp, "%d", &hmm->state_num );

         for( i = 0 ; i < hmm->state_num ; i++ )
            fscanf(fp, "%lf", &( hmm->pi[i] ) );
      }
      else if( strcmp( token, "transition:" ) == 0 ){
         fscanf(fp, "%d", &hmm->state_num );

         for( i = 0 ; i < hmm->state_num ; i++ )
            for( j = 0 ; j < hmm->state_num ; j++ )
               fscanf(fp, "%lf", &( hmm->a[i][j] ));
      }
      else if( strcmp( token, "observation:" ) == 0 ){
         fscanf(fp, "%d", &hmm->observ_num );

         for( i = 0 ; i < hmm->observ_num; i++ )
            for( j = 0 ; j < hmm->state_num ; j++ )
               fscanf(fp, "%lf", &( hmm->b[j][i]) );
      }
    }
    // for(int i = 0; i < hmm->state_num; i++)  hmm->pi[i] *= 100;
    // for( i = 0 ; i < hmm->state_num ; i++ )
    //         for( j = 0 ; j < hmm->state_num ; j++ ) hmm->a[i][j] *= 100;
    // for( i = 0 ; i < hmm->observ_num; i++ )
    //         for( j = 0 ; j < hmm->state_num ; j++ ) hmm->b[i][j] *= 100;
}

static void dumpHMM( FILE *fp, HMM *hmm )
{
   int i, j;

   //fprintf( fp, "model name: %s\n", hmm->model_name );
   fprintf( fp, "initial: %d\n", hmm->state_num );
   for( i = 0 ; i < hmm->state_num; i++ )
      fprintf( fp, "%.5lf%c", hmm->pi[i], " \n"[i == hmm->state_num-1]);

   fprintf( fp, "\ntransition: %d\n", hmm->state_num );
   for( i = 0 ; i < hmm->state_num ; i++ ){
      for( j = 0 ; j < hmm->state_num ; j++ )
         fprintf( fp, "%.5lf%c", hmm->a[i][j], " \n"[j == hmm->state_num-1]);
   }

   fprintf( fp, "\nobservation: %d\n", hmm->observ_num);
   for( i = 0 ; i < hmm->observ_num; i++ ){
      for( j = 0 ; j < hmm->state_num; j++ )
         fprintf( fp, "%.5lf%c", hmm->b[j][i], " \n"[j == hmm->state_num-1]);
   }

}

static int load_models( const char *listname, HMM* hmm, const int max_num )
{
   FILE *fp = open_or_die( listname, "r" );
   int count = 0;
   char filename[MAX_TOTAL_SEQ] = "";
   while( fscanf(fp, "%s", filename) == 1 ){
      loadHMM( &hmm[count], filename );
      count ++;

      if( count >= max_num ){
         return count;
      }
   }
   fclose(fp);

   return count;
}

static void dump_models( HMM *hmm, const int num )
{
   int i = 0;
   for( ; i < num ; i++ ){ 
      //        FILE *fp = open_or_die( hmm[i].model_name, "w" );
      dumpHMM( stderr, &hmm[i] );
   }
}

static void loadSeq(char seq[MAX_TOTAL_SEQ][MAX_SEQ_LEN],const char* filename){
    FILE* fp = open_or_die( filename, "r" );
    //fscanf(fp, "%s", (char*)seq[0]);
    //fscanf(fp, "%s", seq[1]);
    for(int i = 0; fscanf(fp, "%s", (char*)&seq[i]) != EOF; i++){
        if(i >= MAX_TOTAL_SEQ-1){printf("error");break;}
        //else printf("i = %d\n", i);
    //printf("ok\n");
    }
}

static void dumpSeq(char seq[MAX_TOTAL_SEQ][MAX_SEQ_LEN]){
    for(int i = 0; seq[i][0] != '\0'; i++){
        printf("seq%d: %s\n", i, seq[i]);
    }
}




void train(HMM* hmm, char seq[MAX_TOTAL_SEQ][MAX_SEQ_LEN], int batchSize, int totalSeq){
    //printf("check\n");
    int seq_length = strlen(seq[0]);

    for(int batch = 0; batch < (totalSeq/batchSize)+(totalSeq%batchSize!=0); batch++){
        int startSeq = batch*batchSize, endSeq = startSeq + ((totalSeq-startSeq) >= batchSize? batchSize : (totalSeq-startSeq));
        for(int n = startSeq; n < endSeq; n++){
            for(int i = 0; i < hmm->state_num; i++){
                alpha[n][0][i] = hmm->pi[i] * hmm->b[i][seq[n][0] - 'A'];
                //printf("%.5lf\n", hmm->b[i][seq[n][0] - 'A']);
            }
            for(int t = 1; t < seq_length; t++){
                for(int i = 0; i < hmm->state_num; i++){
                    alpha[n][t][i] = 0;
                    for(int j = 0; j < hmm->state_num; j++){
                        alpha[n][t][i] += (alpha[n][t-1][j] * hmm->a[j][i]);
                    }
                    alpha[n][t][i] *= hmm->b[i][seq[n][t] - 'A'];
                }
            }
            for(int i = 0; i < hmm->state_num; i++){
                beta[n][seq_length-1][i] = 1;
            }
            for(int t = seq_length-2; t >=0; t--){
                for(int i = 0; i < hmm->state_num; i++){
                    beta[n][t][i] = 0;
                    for(int j = 0; j < hmm->state_num; j++){
                        beta[n][t][i] += hmm->a[i][j] * hmm->b[j][seq[n][t+1]-'A'] * beta[n][t+1][j];
                    }
                }
            }

            for(int t = 0; t < seq_length; t++){
                double total = 0.0;
                for(int i = 0; i < hmm->state_num; i++){
                    garma[n][t][i] = (alpha[n][t][i]*beta[n][t][i]);
                    total += garma[n][t][i];
                }

                for(int i = 0; i < hmm->state_num; i++){
                    garma[n][t][i] /= total;
                }
            }

            for(int t = 0; t < seq_length-1; t++){
                double total = 0.0;
                for(int i = 0; i < hmm->state_num; i++){
                    for(int j = 0; j < hmm->state_num; j++){
                        epsilon[n][t][i][j] = alpha[n][t][i] * hmm->a[i][j] * hmm->b[j][seq[n][t+1]-'A'] * beta[n][t+1][j];
                        total += epsilon[n][t][i][j];
                    }
                }
                for(int i = 0; i < hmm->state_num; i++){
                    for(int j = 0; j < hmm->state_num; j++){
                        //if(total == 0) total += 0.0000000000001;
                        epsilon[n][t][i][j] /= total;
                    }
                }
            }
        }

        for(int i = 0; i < hmm->state_num; i++){
            double total = 0.0;
            for(int n = startSeq; n < endSeq; n++){
                total += garma[n][0][i];
            }
            hmm->pi[i] = total/batchSize;
        }

        for(int i = 0; i < hmm->state_num; i++){
            for(int j = 0; j < hmm->state_num; j++){
                double num = 0.0, del = 0.0;
                for(int n = startSeq; n < endSeq; n++){
                    for(int t = 0; t < seq_length-2; t++){
                        num += epsilon[n][t][i][j];
                        del += garma[n][t][i];
                    }
                }
                //if(del == 0) del += 0.0000000000001;
                hmm->a[i][j] = num/del;
            }
        }
        for(int i = 0; i < hmm->state_num; i++){
            for(int o = 0; o < hmm->observ_num; o++){
                double del = 0.0, num = 0.0;
                for(int n = startSeq; n < endSeq; n++){
                    for(int t = 0; t < seq_length; t++){
                        num += garma[n][t][i] * ((seq[n][t]-'A') == o);
                        del += garma[n][t][i];
                    }
                }
                hmm->b[i][o] = num/del;
            }
        }
    }
    
}

static double viterbi(HMM* hmm, char seq[MAX_SEQ_LEN]){
    int seq_length = strlen(seq);
    int best_state_seq[MAX_SEQ_LEN];
    double delta[MAX_SEQ_LEN][MAX_STATE];
    int bst[MAX_SEQ_LEN][MAX_STATE];
    //printf("%s\n", seq);
    for(int i = 0; i < hmm->state_num; i++){
        delta[0][i] = hmm->pi[i] * hmm->b[i][seq[0] - 'A'];
    }
    //printf("%d\n", hmm->state_num);
    for(int t = 1; t < seq_length; t++){
        for(int i = 0; i < hmm->state_num; i++){
            //printf("i = %d\n", i);
            delta[t][i] = 0;
            for(int j = 0; j < hmm->state_num; j++){
                double prob = delta[t-1][j]*hmm->a[j][i];
                //printf("prob: %.5lf\n", prob);
                if (prob > delta[t][i]) delta[t][i] = prob, bst[t][i] = j;
            }
            delta[t][i] *= hmm->b[i][seq[t]-'A'];
        }
    }
    double best_delta = delta[seq_length-1][0];
    for(int i = 1; i < hmm->state_num; i++){
        if (delta[seq_length-1][i] > best_delta) best_delta = delta[seq_length-1][i], best_state_seq[seq_length-1] = i; 
    }
    
    for(int t = seq_length-2; t >= 0; t--){
        best_state_seq[t] = bst[t+1][best_state_seq[t+1]];
    }
    // #ifdef DEBUG
    // printf("best_state_prob:%.100lf\n", best_delta);
    // printf("best_state_seq: ");
    // for(int i = 0; i < seq_length; i++){
    //     printf("%d", best_state_seq[i]);
    // }
    // printf("\n");
    // #endif
    return best_delta;
}


#endif
