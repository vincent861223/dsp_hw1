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

#ifndef MAX_SEQ
#   define  MAX_SEQ     200
#endif

#ifndef MAX_LINE
#   define MAX_LINE     20480
#endif

#ifndef MAX_FILENAME_LENGTH
#   define MAX_FILENAME_LENGTH 64
#endif

typedef struct{
   char *model_name;                //model name
   int state_num;                   //number of state
   int observ_num;                  //number of b
   double pi[MAX_STATE];            //pi prob.
   double a[MAX_STATE][MAX_STATE];  //a prob.
   double b[MAX_STATE][MAX_OBSERV]; //b prob.
} HMM;

static FILE *open_or_die( const char *filename, const char *ht )
{
   FILE *fp = fopen( filename, ht );
   if( fp == NULL ){
      perror( filename);
      exit(1);
   }

   return fp;
}

static void loadHMM( HMM *hmm, const char *filename )
{
   int i, j;
   FILE *fp = open_or_die( filename, "r");

   hmm->model_name = (char *)malloc( sizeof(char) * (strlen( filename)+1));
   strcpy( hmm->model_name, filename );

   char token[MAX_LINE] = "";
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
   fprintf( fp, "[PI]: %d states\n", hmm->state_num );
   for( i = 0 ; i < hmm->state_num; i++ )
      fprintf( fp, "state %d: %.5lf\n", i, hmm->pi[i]);

   fprintf( fp, "\n[a]: %d states\n", hmm->state_num );
   for( i = 0 ; i < hmm->state_num ; i++ ){
      for( j = 0 ; j < hmm->state_num ; j++ )
         fprintf( fp, "%.5lf%c", hmm->a[i][j], " \n"[j == hmm->state_num-1]);
   }

   fprintf( fp, "\n[b]: %d states %d observ_nums\n", hmm->state_num, hmm->observ_num);
   for( i = 0 ; i < hmm->state_num; i++ ){
      for( j = 0 ; j < hmm->observ_num; j++ )
         fprintf( fp, "%.5lf%c", hmm->b[i][j], " \n"[j == hmm->observ_num-1]);
   }
}

static int load_models( const char *listname, HMM *hmm, const int max_num )
{
   FILE *fp = open_or_die( listname, "r" );

   int count = 0;
   char filename[MAX_LINE] = "";
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

static void loadSeq(char seq[MAX_LINE][MAX_SEQ], const char* filename){
    FILE* fp = fopen(filename, "r");
    //fscanf(fp, "%s", (char*)seq[0]);
    //fscanf(fp, "%s", seq[1]);
    for(int i = 0; fscanf(fp, "%s", seq[i]) != EOF; i++){
        if(i >= MAX_LINE-1){printf("error");break;}
        //else printf("i = %d\n", i);
    }
}

static void dumpSeq(char seq[MAX_LINE][MAX_SEQ]){
    for(int i = 0; seq[i][0] != '\0'; i++){
        printf("seq%d: %s\n", i, seq[i]);
    }
}

static void calc_alpha(double alpha[MAX_SEQ][MAX_STATE], HMM* hmm, char seq[MAX_SEQ]){
    int seq_length = strlen(seq);
    //printf("seq_length: %d\n", seq_length);
    // t == 0, alpha[t][i] = pi[i] * b[i][o[0]]
    for(int i = 0; i < hmm->state_num; i++){
        alpha[0][i] = hmm->pi[i] * hmm->b[i][seq[0] - 'A'];
        //printf("%.5lf\n", hmm->b[i][seq[0] - 'A']);
    }
    // t > 0
    for(int t = 1; t < seq_length; t++){
        for(int i = 0; i < hmm->state_num; i++){
            alpha[t][i] = 0;
            for(int j = 0; j < hmm->state_num; j++){
                alpha[t][i] += (alpha[t-1][j] * hmm->a[j][i]);
            }
            alpha[t][i] *= hmm->b[i][seq[t] - 'A'];
            //if(alpha[t][i] < 0.0000001) alpha[t][i] = 0.0000001;
            //printf("%.5lf\n", hmm->b[i][seq[t] - 'A']);
        }
    }

    //print alpha
    #ifdef DEBUG
    printf("[alpha]:\n");
    for(int t = 0; t < seq_length; t++){
        for(int i = 0; i < hmm->state_num; i++){
            printf("%.5lf%c", alpha[t][i], " \n"[i == hmm->state_num-1]);
        }
    }
    #endif
}

static void calc_beta(double beta[MAX_SEQ][MAX_STATE], HMM* hmm, char seq[MAX_SEQ]){
    int seq_length = strlen(seq);
    //printf("C : (%c)\n", seq[seq_length-1]);
    // t = T
    for(int i = 0; i < hmm->state_num; i++){
        beta[seq_length-1][i] = 1;
    }
    //printf("check\n");
    // t < T
    for(int t = seq_length-2; t >=0; t--){
        for(int i = 0; i < hmm->state_num; i++){
            beta[t][i] = 0;
            for(int j = 0; j < hmm->state_num; j++){
                beta[t][i] += hmm->a[i][j] * hmm->b[j][seq[t+1]-'A'] * beta[t+1][j];
            }
            //if(beta[t][i] < 0.0000001) beta[t][i] = 0.0000001;
        }
    }
    //print beta
    #ifdef DEBUG
    printf("[beta]:\n");
    for(int t = 0; t < seq_length; t++){
        for(int i = 0; i < hmm->state_num; i++){
            printf("%.5lf%c", beta[t][i], " \n"[i == hmm->state_num-1]);
        }
    }
    #endif
}

static void calc_garma(double garma[MAX_SEQ][MAX_STATE], double alpha[MAX_SEQ][MAX_STATE], double beta[MAX_SEQ][MAX_STATE], HMM* hmm, char seq[MAX_SEQ]){
    int seq_length = strlen(seq);
    for(int t = 0; t < seq_length; t++){
        double total = 0.0;
        for(int i = 0; i < hmm->state_num; i++){
            garma[t][i] = (alpha[t][i]*beta[t][i]);
            if(garma[t][i] == 0.0) printf("error");
            //printf("garma[t][i]: %.5lf\n", garma[t][i]);
            total += garma[t][i];
        }
        //if(total < 0) printf("error\n");
        //printf("total: %.5lf\n", total);
        //if(total == 0) total += 0.0000001;
        for(int i = 0; i < hmm->state_num; i++){
            //printf("%.5lf %.5lf ", garma[t][i], total);
            //if(total == 0) total += 0.0000000000001;

            
            garma[t][i] /= total;
            //printf("%.5lf\n", garma[t][i]);
        }
    }
    #ifdef DEBUG
    printf("[garma]:\n");
    for(int t = 0; t < seq_length; t++){
        for(int i = 0; i < hmm->state_num; i++){
            printf("%.5lf%c", garma[t][i], " \n"[i == hmm->state_num-1]);
        }
    }
    #endif
}

static void calc_epsilon(double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE], double alpha[MAX_SEQ][MAX_STATE], double beta[MAX_SEQ][MAX_STATE], HMM* hmm, char seq[MAX_SEQ]){
    int seq_length = strlen(seq);
    
    for(int t = 0; t < seq_length-1; t++){
        double total = 0.0;
        for(int i = 0; i < hmm->state_num; i++){
            for(int j = 0; j < hmm->state_num; j++){
                epsilon[t][i][j] = alpha[t][i] * hmm->a[i][j] * hmm->b[j][seq[t+1]-'A'] * beta[t+1][j];
                total += epsilon[t][i][j];
            }
        }
        for(int i = 0; i < hmm->state_num; i++){
            for(int j = 0; j < hmm->state_num; j++){
                //if(total == 0) total += 0.0000000000001;
                epsilon[t][i][j] /= total;
            }
        }
    }
    // printf("[epsilon]:\n");
    // for(int t = 0; t < seq_length; t++){
    //     for(int i = 0; i < hmm->state_num; i++){
    //         printf("%.5lf%c", garma[t][i], " \n"[i == hmm->state_num-1]);
    //     }
    // }
}

static void update_pi(double garma[MAX_SEQ][MAX_STATE], HMM* hmm){
    for(int i = 0; i < hmm->state_num; i++){
        hmm->pi[i] = garma[0][i];
    }
}

static void update_a(double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE], double garma[MAX_SEQ][MAX_STATE], HMM* hmm, char seq[MAX_SEQ]){
    int seq_length = strlen(seq);
    for(int i = 0; i < hmm->state_num; i++){
        for(int j = 0; j < hmm->state_num; j++){
            double num = 0, del = 0;
            for(int t = 0; t < seq_length-2; t++){
                num += epsilon[t][i][j];
                del += garma[t][i];
            }
            //if(del == 0) del += 0.0000000000001;
            hmm->a[i][j] = num/del;
        }
    }
}

static void viterbi(int best_state_seq[MAX_SEQ], HMM* hmm, char seq[MAX_SEQ]){
    int seq_length = strlen(seq);
    double delta[MAX_SEQ][MAX_STATE];
    int bst[MAX_SEQ][MAX_STATE];
    for(int i = 0; i < hmm->state_num; i++){
        delta[0][i] = hmm->pi[i] * hmm->b[i][seq[0] - 'A'];
    }

    for(int t = 1; t < seq_length; t++){
        for(int i = 0; i < hmm->state_num; i++){
            delta[t][i] = 0;
            for(int j = 0; j <= i; j++){
                double prob = delta[t-1][j]*hmm->a[j][i];
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

    //#ifdef DEBUG
    printf("best_state_prob:%.100lf\n", best_delta);
    printf("best_state_seq: ");
    for(int i = 0; i < seq_length; i++){
        printf("%d", best_state_seq[i]);
    }
    printf("\n");
    //#endif
}


static void update_b(int best_state_seq[MAX_SEQ], double garma[MAX_SEQ][MAX_STATE], HMM* hmm, char seq[MAX_SEQ]){
    int seq_length = strlen(seq);
    int count[MAX_STATE];
    int sub_count[MAX_STATE][MAX_OBSERV];
    for(int i = 0; i < hmm->state_num; i++){
        for(int o = 0; o < hmm->observ_num; o++){
            double del = 0.0, num = 0.0;
            for(int t = 0; t < seq_length; t++){
                num += garma[t][i] * ((seq[t]-'A') == o);
                del += garma[t][i];
            }
            hmm->b[i][o] = num/del;
        }
    }
    // for(int i = 0; i < MAX_STATE; i++) count[i] = 0;
    // for(int i = 0; i < MAX_STATE; i++){
    //     for(int j = 0; j < MAX_STATE; j++) sub_count[i][j] = 0;
    // }
    // for(int t = 0; t < seq_length; t++){
    //     count[best_state_seq[t]]++;
    //     sub_count[best_state_seq[t]][seq[t]-'A']++;
    // }

    // for(int i = 0; i < hmm->state_num; i++){
    //     #ifdef DEBUG
    //     printf("state%d: ", i);
    //     #endif
    //     if(count[i] == 0) count[i] = hmm->observ_num;
    //     for(int o = 0; o < hmm->observ_num; o++){
    //         hmm->b[i][o] = sub_count[i][o] / (double)count[i];
    //         #ifdef DEBUG
    //         printf("%.5lf(%.5lf/%.5lf)%c", hmm->b[i][o], sub_count[i][o], (double)count[i], " \n"[o == hmm->observ_num-1]);
    //         #endif
    //     }
    // }
}




#endif
